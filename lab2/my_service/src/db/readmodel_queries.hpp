#pragma once

#include <userver/storages/postgres/query.hpp>

namespace myservice::db {

inline const userver::storages::postgres::Query kGetReadModelRecordById{
    R"~(
      SELECT record_id,
             patient_id,
             doctor_id,
             diagnosis,
             notes,
             to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD"T"HH24:MI:SS"Z"') AS created_at
      FROM medical_records_read
      WHERE record_id = $1
    )~",
    userver::storages::postgres::Query::Name{"get_readmodel_record_by_id"}};

}  // namespace myservice::db