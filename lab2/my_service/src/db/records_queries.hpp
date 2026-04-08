#pragma once

#include <userver/storages/postgres/query.hpp>

namespace myservice::db {

inline const userver::storages::postgres::Query kInsertRecordToPatient{
    R"~(
      INSERT INTO medical_records(patient_id, doctor_id, diagnosis, notes)
      VALUES ($1, $2, $3, $4)
      RETURNING id
    )~",
    userver::storages::postgres::Query::Name{"insert_record_to_patient"}};

inline const userver::storages::postgres::Query kSelectPatientRecords{
    R"~(
      SELECT id,
             patient_id,
             doctor_id,
             diagnosis,
             notes,
             to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD"T"HH24:MI:SS"Z"') AS created_at
      FROM medical_records
      WHERE patient_id = $1
      ORDER BY created_at DESC, id DESC
      LIMIT $2 OFFSET $3
    )~",
    userver::storages::postgres::Query::Name{"select_patient_records"}};

inline const userver::storages::postgres::Query kCountPatientRecords{
    R"~(
      SELECT COUNT(*)
      FROM medical_records
      WHERE patient_id = $1
    )~",
    userver::storages::postgres::Query::Name{"count_patient_records"}};

inline const userver::storages::postgres::Query kGetRecordById{
    R"~(
      SELECT id,
             patient_id,
             doctor_id,
             diagnosis,
             notes,
             to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD"T"HH24:MI:SS"Z"') AS created_at
      FROM medical_records
      WHERE id = $1
    )~",
    userver::storages::postgres::Query::Name{"get_record_by_id"}};

}  // namespace myservice::db