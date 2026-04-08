#include "patients_records_create.hpp"

#include <cstdint>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../auth/authz.hpp"
#include "../db/records_queries.hpp"

namespace myservice::handlers {

namespace {

std::int64_t ParseIdPathArgOrThrow(const userver::server::http::HttpRequest& request,
                                  const char* name) {
  try {
    return std::stoll(request.GetPathArg(name));
  } catch (...) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Invalid path parameter"});
  }
}

}  // namespace

PatientRecordsCreate::PatientRecordsCreate(const userver::components::ComponentConfig& config,
                                           const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      auth_cfg_(context.FindComponent<myservice::components::AuthConfig>()) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value PatientRecordsCreate::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext&) const {

  // По варианту: запись делает врач (или admin)
  const auto claims = myservice::auth::VerifyRequestAndGetClaimsOrThrow(request, auth_cfg_);
  myservice::auth::RequireRoleOrThrow(claims.role, {"doctor", "admin"});

  const auto patient_id = ParseIdPathArgOrThrow(request, "patientId");

  if (!json.HasMember("diagnosis") || !json.HasMember("notes")) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Missing fields: diagnosis, notes"});
  }

  const auto diagnosis = json["diagnosis"].As<std::string>();
  const auto notes = json["notes"].As<std::string>();

  const auto res = pg_->Execute(
      userver::storages::postgres::ClusterHostType::kMaster,
      myservice::db::kInsertRecordToPatient,
      patient_id, claims.user_id, diagnosis, notes);

  const auto record_id = res.AsSingleRow<std::int64_t>();

  userver::formats::json::ValueBuilder vb;
  vb["recordId"] = record_id;
  vb["patientId"] = patient_id;
  vb["doctorId"] = claims.user_id;
  return vb.ExtractValue();
}

}  // namespace myservice::handlers