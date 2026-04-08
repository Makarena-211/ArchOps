-- BEGIN;

-- CREATE EXTENSION IF NOT EXISTS pg_trgm;

-- CREATE TABLE IF NOT EXISTS users (
--   id BIGSERIAL PRIMARY KEY,
--   email TEXT NOT NULL UNIQUE,
--   password_hash TEXT NOT NULL,
--   role TEXT NOT NULL DEFAULT 'patient',
--   created_at TIMESTAMPTZ NOT NULL DEFAULT now(),
--   CONSTRAINT chk_users_role CHECK (role IN ('patient','doctor','admin')),
--   CONSTRAINT chk_users_email_nonempty CHECK (length(trim(email)) > 0)
-- );

-- CREATE TABLE IF NOT EXISTS patients (
--   id BIGSERIAL PRIMARY KEY,
--   user_id BIGINT NOT NULL UNIQUE,
--   first_name TEXT NOT NULL,
--   last_name TEXT NOT NULL,
--   middle_name TEXT,
--   birth_date DATE,
--   created_at TIMESTAMPTZ NOT NULL DEFAULT now(),

--   CONSTRAINT fk_patients_user
--     FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,

--   CONSTRAINT chk_patients_first_name_nonempty CHECK (length(trim(first_name)) > 0),
--   CONSTRAINT chk_patients_last_name_nonempty CHECK (length(trim(last_name)) > 0)
-- );

-- CREATE TABLE IF NOT EXISTS medical_records (
--   id BIGSERIAL PRIMARY KEY,
--   patient_id BIGINT NOT NULL,
--   doctor_id BIGINT NOT NULL,
--   diagnosis TEXT NOT NULL,
--   notes TEXT NOT NULL,
--   created_at TIMESTAMPTZ NOT NULL DEFAULT now(),

--   CONSTRAINT fk_records_patient
--     FOREIGN KEY (patient_id) REFERENCES patients(id) ON DELETE CASCADE,

--   CONSTRAINT fk_records_doctor
--     FOREIGN KEY (doctor_id) REFERENCES users(id) ON DELETE RESTRICT,

--   CONSTRAINT chk_records_diagnosis_nonempty CHECK (length(trim(diagnosis)) > 0)
-- );

-- -- -- Indexes
-- -- CREATE INDEX IF NOT EXISTS idx_patients_user_id ON patients(user_id);

-- -- CREATE INDEX IF NOT EXISTS gin_patients_first_name_trgm
-- --   ON patients USING GIN (first_name gin_trgm_ops);
-- -- CREATE INDEX IF NOT EXISTS gin_patients_last_name_trgm
-- --   ON patients USING GIN (last_name gin_trgm_ops);
-- -- CREATE INDEX IF NOT EXISTS gin_patients_fullname_trgm
-- --   ON patients USING GIN ((last_name || ' ' || first_name || ' ' || COALESCE(middle_name, '')) gin_trgm_ops);

-- -- CREATE INDEX IF NOT EXISTS idx_medical_records_patient_id ON medical_records(patient_id);
-- -- CREATE INDEX IF NOT EXISTS idx_medical_records_doctor_id ON medical_records(doctor_id);
-- -- CREATE INDEX IF NOT EXISTS idx_medical_records_patient_created_at
-- --   ON medical_records(patient_id, created_at DESC);

-- COMMIT;