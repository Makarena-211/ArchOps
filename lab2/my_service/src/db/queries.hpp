#pragma once

#include <userver/storages/postgres/query.hpp>

namespace myservice::db {

inline const userver::storages::postgres::Query kInsertRecord{
    R"~(
      INSERT INTO medical_records(patient_id, doctor_id, diagnosis, notes)
      VALUES ($1, $2, $3, $4)
      RETURNING id
    )~",
    userver::storages::postgres::Query::Name{"insert_medical_record"}
};

}  // namespace myservice::db