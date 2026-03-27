#pragma once

#include <userver/storages/postgres/query.hpp>

namespace myservice::db {

inline const userver::storages::postgres::Query kInsertRecord{
    R"~(
      INSERT INTO medical_records(patient_id, doctor_id, diagnosis, notes)
      VALUES ($1, $2, $3, $4)
      RETURNING id
    )~",
    userver::storages::postgres::Query::Name{"insert_medical_record"}};

inline const userver::storages::postgres::Query kUpdateRecordPut{
    R"~(
      UPDATE medical_records
      SET diagnosis = $2,
          notes     = $3
      WHERE id = $1
      RETURNING id
    )~",
    userver::storages::postgres::Query::Name{"update_medical_record_put"}};

inline const userver::storages::postgres::Query kUpdateRecordPatch{
    R"~(
      UPDATE medical_records
      SET diagnosis = COALESCE($2, diagnosis),
          notes     = COALESCE($3, notes)
      WHERE id = $1
      RETURNING id
    )~",
    userver::storages::postgres::Query::Name{"update_medical_record_patch"}};

inline const userver::storages::postgres::Query kDeleteRecord{
    R"~(
      DELETE FROM medical_records
      WHERE id = $1
      RETURNING id
    )~",
    userver::storages::postgres::Query::Name{"delete_medical_record"}};

inline const userver::storages::postgres::Query kSelectAllRecords{
    R"~(
      SELECT id,
            patient_id,
            doctor_id,
            diagnosis,
            notes,
            to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD"T"HH24:MI:SS"Z"') AS created_at
      FROM medical_records
      ORDER BY id DESC
      LIMIT $1 OFFSET $2
    )~",
    userver::storages::postgres::Query::Name{"select_all_medical_records"}};

inline const userver::storages::postgres::Query kCountAllRecords{
    R"~(
      SELECT COUNT(*)
      FROM medical_records
    )~",
    userver::storages::postgres::Query::Name{"count_all_medical_records"}};

}  // namespace myservice::db

