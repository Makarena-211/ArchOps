#include "auth_register.hpp"

#include <string>

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/exceptions.hpp>

#include "../auth/password.hpp"
#include "../db/auth_queries.hpp"

namespace myservice::handlers {

AuthRegister::AuthRegister(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      cfg_(context.FindComponent<myservice::components::AuthConfig>()) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value AuthRegister::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext&) const {
  if (!json.HasMember("email") || !json.HasMember("password")) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Missing fields: email, password"});
  }

  const auto email = json["email"].As<std::string>();
  const auto password = json["password"].As<std::string>();

  if (email.empty() || password.size() < 4) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Invalid email/password"});
  }

  const auto pass_hash = myservice::auth::HashPassword(password, cfg_.GetJwtSecret());

  try {
    const auto res = pg_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                                  myservice::db::kCreateUser,
                                  email, pass_hash, "patient");
    const auto user_id = res.AsSingleRow<std::int64_t>();

    userver::formats::json::ValueBuilder vb;
    vb["userId"] = user_id;
    vb["email"] = email;
    vb["role"] = "patient";
    return vb.ExtractValue();
  } catch (const userver::storages::postgres::UniqueViolation&) {
    throw userver::server::handlers::ConflictError(
        userver::server::handlers::ExternalBody{"User already exists"});
  }
}

}  // namespace myservice::handlers