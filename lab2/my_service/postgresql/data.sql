-- чистим
TRUNCATE TABLE medical_records RESTART IDENTITY CASCADE;
TRUNCATE TABLE patients RESTART IDENTITY CASCADE;
TRUNCATE TABLE users RESTART IDENTITY CASCADE;

-- users (12)
INSERT INTO users(email, password_hash, role) VALUES
  ('admin@example.com',  'hash_admin',  'admin'),
  ('doc1@example.com',   'hash_doc1',   'doctor'),
  ('doc2@example.com',   'hash_doc2',   'doctor'),
  ('doc3@example.com',   'hash_doc3',   'doctor'),
  ('user1@example.com',  'hash_user1',  'patient'),
  ('user2@example.com',  'hash_user2',  'patient'),
  ('user3@example.com',  'hash_user3',  'patient'),
  ('user4@example.com',  'hash_user4',  'patient'),
  ('user5@example.com',  'hash_user5',  'patient'),
  ('user6@example.com',  'hash_user6',  'patient'),
  ('user7@example.com',  'hash_user7',  'patient'),
  ('user8@example.com',  'hash_user8',  'patient');

INSERT INTO users(email, password_hash, role) VALUES
  ('user9@example.com',  'hash_user9',  'patient'),
  ('user10@example.com', 'hash_user10', 'patient');

-- patients (10)
INSERT INTO patients(user_id, first_name, last_name, middle_name, birth_date) VALUES
  (5,  'Иван',     'Иванов',    'Иванович', DATE '1990-01-10'),
  (6,  'Пётр',     'Петров',    'Петрович', DATE '1985-05-20'),
  (7,  'Анна',     'Смирнова',  'Олеговна', DATE '1992-11-03'),
  (8,  'Мария',    'Кузнецова', 'Игоревна', DATE '1979-07-14'),
  (9,  'Сергей',   'Попов',     'Андреевич',DATE '1988-02-28'),
  (10, 'Елена',    'Васильева', 'Сергеевна',DATE '1995-09-09'),
  (11, 'Алексей',  'Морозов',   'Викторович',DATE '1981-12-12'),
  (12, 'Ольга',    'Новикова',  'Павловна', DATE '1993-03-30'),
  (13, 'Никита',   'Фёдоров',   'Дмитриевич',DATE '2000-04-04'),
  (14, 'Татьяна',  'Соколова',  'Ильинична',DATE '1987-06-06');

-- medical_records (20)
INSERT INTO medical_records(patient_id, doctor_id, diagnosis, notes, created_at) VALUES
  (1, 2, 'ОРВИ',          'Покой, питьё',                    now() - interval '20 days'),
  (1, 3, 'Грипп',         'Жаропонижающее',                  now() - interval '10 days'),
  (1, 2, 'Выздоровление', 'Контрольный осмотр',              now() - interval '2 days'),
  (2, 2, 'Гастрит',       'Диета',                           now() - interval '30 days'),
  (2, 4, 'Гастрит',       'ИПП 14 дней',                     now() - interval '15 days'),
  (3, 3, 'Аллергия',      'Антигистаминные',                 now() - interval '25 days'),
  (3, 3, 'Аллергия',      'Повторный приём',                 now() - interval '5 days'),
  (4, 4, 'Гипертония',    'Контроль давления',               now() - interval '60 days'),
  (4, 2, 'Гипертония',    'Коррекция терапии',               now() - interval '35 days'),
  (5, 2, 'Травма',        'Рентген, покой',                  now() - interval '7 days'),
  (5, 2, 'Травма',        'Снятие повязки',                  now() - interval '1 day'),
  (6, 3, 'COVID-19',      'Изоляция',                        now() - interval '90 days'),
  (6, 3, 'Постковид',     'Реабилитация',                    now() - interval '70 days'),
  (7, 4, 'Ангина',        'Антибиотик',                      now() - interval '14 days'),
  (7, 4, 'Ангина',        'Контроль',                        now() - interval '11 days'),
  (8, 2, 'Мигрень',       'НПВС по требованию',              now() - interval '40 days'),
  (8, 3, 'Мигрень',       'Дневник головной боли',           now() - interval '12 days'),
  (9, 4, 'Дерматит',      'Мазь',                            now() - interval '18 days'),
  (9, 2, 'Дерматит',      'Контроль',                        now() - interval '8 days'),
  (10,3, 'Профосмотр',    'Рекомендации',                    now() - interval '3 days');