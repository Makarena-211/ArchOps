`my_service` — учебный REST API сервис на **userver** с хранением данных в **PostgreSQL**. Тематика — “медицинское учреждение”: регистрация пользователей и управление медицинскими записями.

---

# 1) Реализованный функционал (API)

## 1.1 Системные эндпоинты
### `GET /ping`
Проверка доступности сервиса.

**Ответ**: `200 OK`.

### `GET /health`
Health-check сервиса.

**Ответ**: `200 OK`
```json
{ "status": "ok" }
```

---

## 1.2 Аутентификация
Используется JWT Bearer (токен передаётся в заголовке `Authorization`).

### `POST /api/auth/register`
Публичная регистрация пользователя.

**Request JSON**
```json
{
  "email": "user@example.com",
  "password": "1234"
}
```

**Response JSON**
```json
{
  "userId": 1,
  "email": "user@example.com",
  "role": "patient"
}
```

Ошибки:
- `400` — некорректные данные
- `409` — пользователь уже существует

### `POST /api/auth/login`
Авторизация по email+password, выдача JWT.

**Request JSON**
```json
{
  "email": "user@example.com",
  "password": "1234"
}
```

**Response JSON**
```json
{
  "accessToken": "<JWT>",
  "tokenType": "Bearer",
  "expiresIn": 3600
}
```

Ошибки:
- `400` — некорректные данные
- `401` — неверные credentials

---

## 1.3 Медицинские записи (CRUD)
Записи хранятся в таблице `medical_records`.

### `POST /api/records`
Создание медицинской записи.

**Требует токен**:
```
Authorization: Bearer <JWT>
```

**Request JSON**
```json
{
  "patientId": 1,
  "doctorId": 2,
  "diagnosis": "Flu",
  "notes": "Rest and drink water"
}
```

**Response JSON**
```json
{ "recordId": 123 }
```

Ошибки:
- `400` — отсутствуют обязательные поля
- `401` — отсутствует/невалидный токен

### `GET /api/records?limit=50&offset=0`
Список медицинских записей (пагинация).

**Response JSON**
```json
{
  "items": [
    {
      "id": 123,
      "patientId": 1,
      "doctorId": 2,
      "diagnosis": "Flu",
      "notes": "Rest",
      "createdAt": "2026-03-27T12:34:56Z"
    }
  ],
  "limit": 50,
  "offset": 0,
  "total": 1
}
```

### `PUT /api/records/{recordId}`
Полное обновление (замена полей `diagnosis`, `notes`).

**Request JSON**
```json
{
  "diagnosis": "Updated diagnosis",
  "notes": "Updated notes"
}
```

**Response JSON**
```json
{
  "recordId": 123,
  "updated": true
}
```

Ошибки:
- `400` — некорректное тело
- `404` — запись не найдена

### `PATCH /api/records/{recordId}`
Частичное обновление (`diagnosis` и/или `notes`).

**Request JSON**
```json
{
  "diagnosis": "Patched diagnosis"
}
```

**Response JSON**
```json
{
  "recordId": 123,
  "patched": true
}
```

Ошибки:
- `400` — нечего обновлять/некорректные поля
- `404` — запись не найдена

### `DELETE /api/records/{recordId}`
Удаление записи.

**Response JSON**
```json
{
  "recordId": 123,
  "deleted": true
}
```

Ошибки:
- `404` — запись не найдена

---

## 1.4 Роли
- В таблице `users` есть поле `role` (например: `patient`, `doctor`, `admin`).
- Роль попадает в JWT при логине.
- Для тестирования в `main.py` роль пользователя меняется напрямую в БД (через `psycopg2`), затем выполняется повторный логин.

---

# 2) Структура данных (PostgreSQL)

Используются таблицы:
- `users` — пользователи (email, password_hash, role, created_at)
- `medical_records` — медицинские записи (patient_id, doctor_id, diagnosis, notes, created_at)

Миграции лежат в `postgresql/*.sql` (например `001_create_migrations.sql`).

---

# 3) Проверка функционала через Python (`main.py`)
В репозитории есть Python-скрипт `main.py`, который выполняет сценарий:
1) `/ping`, `/health`
2) регистрация пользователя
3) просмотр пользователей в БД
4) смена роли на `doctor` (через БД)
5) логин → получение JWT
6) создание записи → list → put → patch → delete

Зависимости:
```bash
pip install -r requirements.txt
```

---

# 4) Запуск приложения через Docker (без DevContainer)

## 4.1 Требования
- установлен Docker и docker compose plugin
- свободны порты:
  - `8080` (API)
  - `5432` (PostgreSQL, если проброшен наружу)

## 4.2 Запуск
Из корня проекта (где лежат `Dockerfile` и `docker-compose.yaml`):

```bash
docker compose up --build
```

Что произойдёт:
1) поднимется контейнер `postgres`
2) выполнится контейнер `migrations` и применит SQL из `postgresql/`
3) поднимется контейнер `my_service` и запустит API на порту `8080`

Проверка:
```bash
curl http://localhost:8080/health
```
