-- queries.sql
-- SQL-запросы для операций системы

-- 1) Создание нового пользователя (используется в /api/auth/register)
-- params: $1 email, $2 password_hash, $3 role
INSERT INTO users(email, password_hash, role)
VALUES ($1, $2, $3)
RETURNING id;

-- 2) Поиск пользователя по логину (login == email)
-- param: $1 login/email
SELECT id, email, role, created_at
FROM users
WHERE email = $1;

-- 3) Поиск пользователя по маске имя и фамилии (через patient profile)
-- params: $1 mask like '%Иван%', $2 limit, $3 offset
SELECT u.id AS user_id, u.email, u.role,
       p.id AS patient_id, p.first_name, p.last_name, p.middle_name
FROM users u
JOIN patients p ON p.user_id = u.id
WHERE (p.last_name || ' ' || p.first_name || ' ' || COALESCE(p.middle_name,'')) ILIKE $1
   OR p.first_name ILIKE $1
   OR p.last_name ILIKE $1
ORDER BY p.last_name, p.first_name, u.id
LIMIT $2 OFFSET $3;

-- (опционально) count для пагинации
SELECT COUNT(*)
FROM users u
JOIN patients p ON p.user_id = u.id
WHERE (p.last_name || ' ' || p.first_name || ' ' || COALESCE(p.middle_name,'')) ILIKE $1
   OR p.first_name ILIKE $1
   OR p.last_name ILIKE $1;

-- 4) Регистрация пациента (создание профиля пациента)
-- params: $1 user_id, $2 first_name, $3 last_name, $4 middle_name, $5 birth_date
INSERT INTO patients(user_id, first_name, last_name, middle_name, birth_date)
VALUES ($1, $2, $3, $4, $5::date)
RETURNING id;

-- 5) Поиск пациента по ФИО (mask)
-- params: $1 mask like '%Иван%', $2 limit, $3 offset
SELECT p.id AS patient_id, p.user_id, u.email,
       p.first_name, p.last_name, p.middle_name, p.birth_date
FROM patients p
JOIN users u ON u.id = p.user_id
WHERE p.first_name ILIKE $1
   OR p.last_name ILIKE $1
   OR (p.last_name || ' ' || p.first_name || ' ' || COALESCE(p.middle_name,'')) ILIKE $1
ORDER BY p.last_name, p.first_name
LIMIT $2 OFFSET $3;

-- 6) Создание медицинской записи (старый эндпоинт /api/records)
-- params: $1 patient_id, $2 doctor_id, $3 diagnosis, $4 notes
INSERT INTO medical_records(patient_id, doctor_id, diagnosis, notes)
VALUES ($1, $2, $3, $4)
RETURNING id;

-- 7) Добавление записи к пациенту (новый эндпоинт /api/patients/{id}/records)
-- по сути тот же INSERT, doctor_id берём из JWT
INSERT INTO medical_records(patient_id, doctor_id, diagnosis, notes)
VALUES ($1, $2, $3, $4)
RETURNING id;

-- 8) Получение истории записей пациента
-- params: $1 patient_id, $2 limit, $3 offset
SELECT id, patient_id, doctor_id, diagnosis, notes, created_at
FROM medical_records
WHERE patient_id = $1
ORDER BY created_at DESC, id DESC
LIMIT $2 OFFSET $3;

SELECT COUNT(*)
FROM medical_records
WHERE patient_id = $1;

-- 9) Получение записи по коду (id)
-- param: $1 record_id
SELECT id, patient_id, doctor_id, diagnosis, notes, created_at
FROM medical_records
WHERE id = $1;