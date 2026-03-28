#include "auth_login.hpp"

#include <chrono>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../auth/jwt.hpp"
#include "../auth/password.hpp"
#include "../db/auth_queries.hpp"

namespace myservice::handlers {

namespace {
std::int64_t NowUnix() {
  const auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}
}  // namespace

AuthLogin::AuthLogin(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      cfg_(context.FindComponent<myservice::components::AuthConfig>()) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value AuthLogin::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext&) const {
  if (!json.HasMember("email") || !json.HasMember("password")) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Missing fields: email, password"});
  }

  const auto email = json["email"].As<std::string>();
  const auto password = json["password"].As<std::string>();

  const auto res = pg_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                                myservice::db::kGetUserByEmail, email);
  if (res.IsEmpty()) {
    throw userver::server::handlers::Unauthorized(
        userver::server::handlers::ExternalBody{"Invalid credentials"});
  }

  const auto row = res.Front();
  const auto user_id = row["id"].As<std::int64_t>();
  const auto stored_hash = row["password_hash"].As<std::string>();
  const auto role = row["role"].As<std::string>();

  const auto pass_hash = myservice::auth::HashPassword(password, cfg_.GetJwtSecret());
  if (stored_hash != pass_hash) {
    throw userver::server::handlers::Unauthorized(
        userver::server::handlers::ExternalBody{"Invalid credentials"});
  }

  myservice::auth::JwtClaims claims;
  claims.user_id = user_id;
  claims.email = email;
  claims.role = role;
  claims.exp = NowUnix() + cfg_.GetTtl().count();

  const auto token = myservice::auth::CreateTokenHS256(claims, cfg_.GetJwtSecret());

  userver::formats::json::ValueBuilder vb;
  vb["accessToken"] = token;
  vb["tokenType"] = "Bearer";
  vb["expiresIn"] = cfg_.GetTtl().count();
  return vb.ExtractValue();
}

}  // namespace myservice::handlers