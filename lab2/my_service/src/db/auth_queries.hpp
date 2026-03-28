#pragma once

#include <userver/storages/postgres/query.hpp>

namespace myservice::db {

inline const userver::storages::postgres::Query kCreateUser{
    R"~(
      INSERT INTO users(email, password_hash, role)
      VALUES ($1, $2, $3)
      RETURNING id
    )~",
    userver::storages::postgres::Query::Name{"create_user"}};

inline const userver::storages::postgres::Query kGetUserByEmail{
    R"~(
      SELECT id, email, password_hash, role
      FROM users
      WHERE email = $1
    )~",
    userver::storages::postgres::Query::Name{"get_user_by_email"}};

}  // namespace myservice::db