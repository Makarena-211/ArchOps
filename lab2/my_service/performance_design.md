# HW05 — Оптимизация производительности: caching + rate limiting (без Redis)

В рамках ДЗ реализованы:
- **in-memory кеш** в процессе сервиса (без Redis) с TTL и инвалидацией
- **rate limiting** (Token Bucket) для защиты “дорогого” эндпоинта
- документация, обновление docker-compose (Redis убран)

---

## 1. Анализ производительности

### 1.1 Hot paths (частые операции)
1) `GET /api/records/{recordId}` — получение мед. записи по id  
2) `GET /api/patients/{patientId}/records?limit&offset` — история записей пациента  
3) `GET /api/patients/search?mask=...` — поиск по маске (часто используется UI, потенциально “дорогой”)

### 1.2 Потенциально медленные операции (bottlenecks)
- Обращения к PostgreSQL на чтение:
  - `SELECT ... WHERE id = $1`
  - `SELECT ... WHERE patient_id = $1 ORDER BY created_at DESC LIMIT/OFFSET` + `COUNT(*)`
  - `ILIKE '%mask%'` (даже с pg_trgm/GIN может быть существенно тяжелее простых lookup)
- Сетевой hop + парсинг результата запроса
- Пагинация + count может удваивать нагрузку (2 запроса вместо 1)

### 1.3 Целевые требования к производительности (ориентиры)
- P95 latency для простого GET по id:
  - **из кеша**: единицы миллисекунд
  - **из БД**: десятки миллисекунд (зависит от окружения)
- Устойчивость к burst-нагрузкам:
  - уменьшение количества запросов в БД за счёт кеша
  - ограничение частоты запросов к “дорогим” эндпоинтам rate limiting’ом

---

## 2. Стратегия кеширования

### 2.1 Ограничение
Реализован **кеш в памяти процесса**.

Особенности in-memory кеша:
- не шарится между инстансами (в Kubernetes/несколько реплик — кеш у каждой свой)
- сбрасывается при рестарте
- даёт заметный эффект при повторных чтениях и burst‑трафике на один инстанс

### 2.2 Что кешируем (минимум 2 endpoints)
1) `GET /api/records/{recordId}`
   - кешируем JSON ответа записи по ключу `record:{recordId}`
   - TTL: **30 секунд**

2) `GET /api/patients/{patientId}/records?limit&offset`
   - кешируем JSON ответа списка по ключу  
     `patient_records:{patientId}:limit={limit}:offset={offset}`
   - TTL: **10 секунд**

### 2.3 Выбранная стратегия
**Cache-Aside (Lazy Loading)**:
- на чтении:
  1) пробуем получить значение из кеша
  2) если промах — читаем из БД и сохраняем в кеш на TTL
- на записи/изменении:
  - инвалидируем связанные ключи

### 2.4 Инвалидация кеша
Инвалидация реализована на изменяющих операциях:

- `POST /api/patients/{patientId}/records`
  - инвалидируем историю пациента:
    - `InvalidateByPrefix("patient_records:{patientId}:")`

- `PUT /api/records/{recordId}`
- `PATCH /api/records/{recordId}`
- `DELETE /api/records/{recordId}`
  - инвалидируем:
    - `Invalidate("record:{recordId}")`
    - `InvalidateByPrefix("patient_records:{patientId}:")`

Чтобы знать `patientId` при PUT/PATCH/DELETE, перед изменением выполняется запрос:
- `SELECT patient_id FROM medical_records WHERE id=$1`

---

## 3. Реализация кеширования (фактическая)

### 3.1 Компонент кеша
Компонент: `inmemory-cache`  
Реализован как `userver::components::ComponentBase` с настройкой:
- `max-entries` — ограничение количества записей (best-effort)

Хранит:
- `unordered_map<string, Entry{value, expires_at}>`
- mutex для потокобезопасности
- счётчики hits/misses (можно использовать для hit-rate)

#### Важная правка после первой версии
Так как handler’ы используют кеш через `const myservice::components::InMemoryCache& cache_`, а `HandleRequestJsonThrow` — `const`, методы кеша, которые меняют состояние, должны быть **`const`**:
- `Put(...) const`
- `Invalidate(...) const`
- `InvalidateByPrefix(...) const`

Состояние внутри компонента помечено `mutable`, что разрешает изменения под мьютексом.

---

## 4. Проектирование rate limiting

### 4.1 Что ограничиваем
Эндпоинт: `GET /api/patients/search`  
Причины:
- потенциально “дорогой” запрос (ILIKE/trgm)
- хорошо защищает БД от скрейпинга/перебора/всплесков нагрузки

### 4.2 Алгоритм
**Token Bucket**
- `capacity` — максимальный размер ведра (количество токенов)
- `refill-per-second` — скорость пополнения
- каждый запрос потребляет 1 токен
- при отсутствии токенов → HTTP **429 Too Many Requests**

### 4.3 Лимит
- **60 запросов в минуту**  
Реализовано как:
- `capacity: 60`
- `refill-per-second: 1` (≈ 60/min)

### 4.4 Ключ rate limiting (идентификатор клиента)
Порядок выбора ключа:
1) `X-Client-Id` (если клиент прислал)
2) `X-Forwarded-For` (если есть)
3) `X-Real-IP` (если есть)
4) fallback: `"unknown"`

#### Важная правка после первой версии
В данной версии userver `request.GetRemoteAddress()` возвращает `Sockaddr`, но у него **нет `ToString()`**, поэтому IP берётся из заголовков прокси. Это стабильнее и совместимо.

### 4.5 Заголовки rate limiting
Добавляются в ответ всегда, когда rate limiting включён:
- `X-RateLimit-Limit` — лимит (capacity)
- `X-RateLimit-Remaining` — остаток токенов
- `X-RateLimit-Reset` — через сколько секунд будет доступен следующий токен (приближение)

---

## 5. Реализация rate limiting (фактическая)

### 5.1 Компонент RateLimiter
Компонент: `rate-limiter`  
Хранит in-memory бакеты `key -> {tokens, last_time}` под мьютексом.

#### Важные правки после первой версии
1) Метод `Check(...)` сделан **`const`**, так как handler хранит `const RateLimiter& rl_`, а обработчик — `const`.
2) Из-за строгого статического валидатора схем в userver:
   - в `GetStaticConfigSchema()` добавлены **description** для узлов (`default`, `capacity`, `refill-per-second`)
3) Конфиг упрощён:
   - убраны per-endpoint overrides (`endpoints:`) из схемы и из `static_config.yaml`,
     чтобы не ловить дополнительные требования валидатора схемы на вложенные объекты.
   - endpoint_name в `Check(endpoint_name, key)` сейчас не влияет на лимиты (используется default).

### 5.2 Включение rate limiting в handler
В handler `PatientsSearch` добавлено кастомное поле конфига:
- `rate-limit-name` (строка; пустая строка отключает rate limit)

#### Важная правка после первой версии
Так как userver валидирует handler-конфиг по схеме, поле `rate-limit-name` должно быть описано:
- добавлен `PatientsSearch::GetStaticConfigSchema()`, где объявлено поле `rate-limit-name`

### 5.3 Возврат 429
В данной версии userver нет удобного исключения вида `handlers::TooManyRequests`, поэтому сделано так:
- `request.GetHttpResponse().SetStatus(userver::http::StatusCode::kTooManyRequests)`
- возвращается JSON:
```json
{ "message": "Too Many Requests" }
```

---

## 6. Анализ влияния на производительность

### 6.1 Как кеш улучшает производительность
- снижает количество запросов в PostgreSQL на hot paths
- сокращает latency повторных GET (ответ из памяти)
- стабилизирует работу при burst‑нагрузке (часть запросов не доходит до БД)

### 6.2 Как rate limiting улучшает производительность
- ограничивает дорогие запросы (`patients/search`) и защищает БД от перегрузки
- снижает риск деградации latency для остальных эндпоинтов
- повышает устойчивость системы к злоупотреблениям (скрейпинг/подбор)

---

## 7. Метрики и мониторинг

### 7.1 Кеш
- Cache hit rate:
  - `hit_rate = hits / (hits + misses)`
- размер кеша (кол-во entries)
- количество инвалидций

Практически:
- hits/misses хранятся в компоненте `InMemoryCache` (счётчики)

### 7.2 Rate limiting
- количество ответов 429
- распределение по ключам (какие клиенты чаще всего ограничиваются)

### 7.3 Общие
- latency P50/P95/P99 по эндпоинтам
- RPS по эндпоинтам
- метрики PostgreSQL: время запросов, активные коннекты, statement timeout, locks