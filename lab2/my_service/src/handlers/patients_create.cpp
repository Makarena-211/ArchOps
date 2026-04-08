#include "patients_create.hpp"

#include <optional>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/exceptions.hpp>

#include "../auth/authz.hpp"
#include "../db/patient_queries.hpp"

namespace myservice::handlers {

PatientsCreate::PatientsCreate(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      auth_cfg_(context.FindComponent<myservice::components::AuthConfig>()) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value PatientsCreate::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext&) const {

  const auto claims = myservice::auth::VerifyRequestAndGetClaimsOrThrow(request, auth_cfg_);
  myservice::auth::RequireRoleOrThrow(claims.role, {"patient", "admin"});

  if (!json.HasMember("firstName") || !json.HasMember("lastName")) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Missing fields: firstName, lastName"});
  }

  const auto first = json["firstName"].As<std::string>();
  const auto last = json["lastName"].As<std::string>();
  const std::optional<std::string> middle =
      json.HasMember("middleName") ? std::optional<std::string>{json["middleName"].As<std::string>()}
                                   : std::nullopt;
  const std::optional<std::string> birth =
      json.HasMember("birthDate") ? std::optional<std::string>{json["birthDate"].As<std::string>()}
                                  : std::nullopt;

  if (first.empty() || last.empty()) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"firstName/lastName must not be empty"});
  }

  try {
    const auto res = pg_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        myservice::db::kCreatePatient,
        claims.user_id,
        first,
        last,
        middle,
        birth);

    const auto patient_id = res.AsSingleRow<std::int64_t>();

    userver::formats::json::ValueBuilder vb;
    vb["patientId"] = patient_id;
    vb["userId"] = claims.user_id;
    vb["firstName"] = first;
    vb["lastName"] = last;
    if (middle) vb["middleName"] = *middle;
    if (birth) vb["birthDate"] = *birth;
    return vb.ExtractValue();
  } catch (const userver::storages::postgres::UniqueViolation&) {
    throw userver::server::handlers::ConflictError(
        userver::server::handlers::ExternalBody{"Patient profile already exists for this user"});
  }
}

}  // namespace myservice::handlers