#pragma once

#include <userver/storages/postgres/query.hpp>

namespace myservice::db {

inline const userver::storages::postgres::Query kCreatePatient{
    R"~(
      INSERT INTO patients(user_id, first_name, last_name, middle_name, birth_date)
      VALUES ($1, $2, $3, $4, $5::date)
      RETURNING id
    )~",
    userver::storages::postgres::Query::Name{"create_patient"}};

inline const userver::storages::postgres::Query kSearchPatientsByMask{
    R"~(
      SELECT p.id        AS patient_id,
             p.user_id   AS user_id,
             u.email     AS email,
             p.first_name,
             p.last_name,
             p.middle_name,
             to_char(p.birth_date, 'YYYY-MM-DD') AS birth_date
      FROM patients p
      JOIN users u ON u.id = p.user_id
      WHERE p.first_name ILIKE $1
         OR p.last_name ILIKE $1
         OR (p.last_name || ' ' || p.first_name || ' ' || COALESCE(p.middle_name, '')) ILIKE $1
      ORDER BY p.last_name, p.first_name
      LIMIT $2 OFFSET $3
    )~",
    userver::storages::postgres::Query::Name{"search_patients_by_mask"}};

}  // namespace myservice::db