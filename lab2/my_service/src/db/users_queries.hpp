#pragma once

#include <userver/storages/postgres/query.hpp>

namespace myservice::db {

inline const userver::storages::postgres::Query kGetUserByLogin{
    R"~(
      SELECT id, email, role,
             to_char(created_at AT TIME ZONE 'UTC', 'YYYY-MM-DD"T"HH24:MI:SS"Z"') AS created_at
      FROM users
      WHERE email = $1
    )~",
    userver::storages::postgres::Query::Name{"get_user_by_login"}};

// Поиск "пользователей по ФИО" — через patients (если профиля пациента нет, user не попадёт в выдачу)
inline const userver::storages::postgres::Query kSearchUsersByFullNameMask{
    R"~(
      SELECT u.id AS user_id,
             u.email,
             u.role,
             p.id AS patient_id,
             p.first_name,
             p.last_name,
             p.middle_name
      FROM users u
      JOIN patients p ON p.user_id = u.id
      WHERE (p.last_name  || ' ' || p.first_name || ' ' || COALESCE(p.middle_name,'')) ILIKE $1
         OR p.first_name ILIKE $1
         OR p.last_name ILIKE $1
      ORDER BY p.last_name, p.first_name, u.id
      LIMIT $2 OFFSET $3
    )~",
    userver::storages::postgres::Query::Name{"search_users_by_fullname_mask"}};

inline const userver::storages::postgres::Query kCountUsersByFullNameMask{
    R"~(
      SELECT COUNT(*)
      FROM users u
      JOIN patients p ON p.user_id = u.id
      WHERE (p.last_name  || ' ' || p.first_name || ' ' || COALESCE(p.middle_name,'')) ILIKE $1
         OR p.first_name ILIKE $1
         OR p.last_name ILIKE $1
    )~",
    userver::storages::postgres::Query::Name{"count_users_by_fullname_mask"}};

}  // namespace myservice::db