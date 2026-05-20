CREATE EXTENSION IF NOT EXISTS pg_trgm;

-- =========================
-- TABLE: users
-- =========================
CREATE TABLE IF NOT EXISTS users (
  id            BIGSERIAL PRIMARY KEY,
  email         TEXT NOT NULL UNIQUE,
  password_hash TEXT NOT NULL,
  role          TEXT NOT NULL DEFAULT 'patient',
  created_at    TIMESTAMPTZ NOT NULL DEFAULT now(),

  CONSTRAINT chk_users_role
    CHECK (role IN ('patient','doctor','admin')),

  CONSTRAINT chk_users_email_nonempty
    CHECK (length(trim(email)) > 0),

  CONSTRAINT chk_users_password_hash_nonempty
    CHECK (length(trim(password_hash)) > 0)
);

-- =========================
-- TABLE: patients
-- =========================
CREATE TABLE IF NOT EXISTS patients (
  id          BIGSERIAL PRIMARY KEY,
  user_id     BIGINT NOT NULL UNIQUE,
  first_name  TEXT NOT NULL,
  last_name   TEXT NOT NULL,
  middle_name TEXT,
  birth_date  DATE,
  created_at  TIMESTAMPTZ NOT NULL DEFAULT now(),

  CONSTRAINT fk_patients_user
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,

  CONSTRAINT chk_patients_first_name_nonempty
    CHECK (length(trim(first_name)) > 0),

  CONSTRAINT chk_patients_last_name_nonempty
    CHECK (length(trim(last_name)) > 0),

  CONSTRAINT chk_patients_birth_date_reasonable
    CHECK (birth_date IS NULL OR birth_date >= DATE '1900-01-01')
);

-- =========================
-- TABLE: medical_records (WRITE MODEL)
-- =========================
CREATE TABLE IF NOT EXISTS medical_records (
  id         BIGSERIAL PRIMARY KEY,
  patient_id BIGINT NOT NULL,
  doctor_id  BIGINT NOT NULL,
  diagnosis  TEXT NOT NULL,
  notes      TEXT NOT NULL,
  created_at TIMESTAMPTZ NOT NULL DEFAULT now(),

  CONSTRAINT fk_records_patient
    FOREIGN KEY (patient_id) REFERENCES patients(id) ON DELETE CASCADE,

  CONSTRAINT fk_records_doctor
    FOREIGN KEY (doctor_id) REFERENCES users(id) ON DELETE RESTRICT,

  CONSTRAINT chk_records_diagnosis_nonempty
    CHECK (length(trim(diagnosis)) > 0),

  CONSTRAINT chk_records_notes_nonempty
    CHECK (length(trim(notes)) > 0)
);

-- =========================
-- TABLE: medical_records_read (READ MODEL, CQRS)
-- =========================
CREATE TABLE IF NOT EXISTS medical_records_read (
  record_id  BIGINT PRIMARY KEY,
  patient_id BIGINT NOT NULL,
  doctor_id  BIGINT NOT NULL,
  diagnosis  TEXT NOT NULL,
  notes      TEXT NOT NULL,
  created_at TIMESTAMPTZ NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_medical_records_read_patient_id
  ON medical_records_read(patient_id);

-- INDEXES

CREATE INDEX IF NOT EXISTS idx_medical_records_patient_id
  ON medical_records(patient_id);

CREATE INDEX IF NOT EXISTS idx_medical_records_doctor_id
  ON medical_records(doctor_id);

CREATE INDEX IF NOT EXISTS idx_medical_records_patient_created_at
  ON medical_records(patient_id, created_at DESC, id DESC);

CREATE INDEX IF NOT EXISTS gin_patients_first_name_trgm
  ON patients USING GIN (first_name gin_trgm_ops);

CREATE INDEX IF NOT EXISTS gin_patients_last_name_trgm
  ON patients USING GIN (last_name gin_trgm_ops);

CREATE INDEX IF NOT EXISTS gin_patients_fullname_trgm
  ON patients USING GIN ((last_name || ' ' || first_name || ' ' || COALESCE(middle_name, '')) gin_trgm_ops);