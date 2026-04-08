#include "users_get_by_login.hpp"

#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../db/users_queries.hpp"

namespace myservice::handlers {

UsersGetByLogin::UsersGetByLogin(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value UsersGetByLogin::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  const auto login = request.GetArg("login");
  if (login.empty()) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Missing query parameter: login"});
  }

  const auto res = pg_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      myservice::db::kGetUserByLogin,
      login);

  if (res.IsEmpty()) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"User not found"});
  }

  const auto row = res.Front();
  userver::formats::json::ValueBuilder out;
  out["userId"] = row["id"].As<std::int64_t>();
  out["login"] = row["email"].As<std::string>();   // login==email
  out["email"] = row["email"].As<std::string>();
  out["role"] = row["role"].As<std::string>();
  out["createdAt"] = row["created_at"].As<std::string>();
  return out.ExtractValue();
}

}  // namespace myservice::handlers