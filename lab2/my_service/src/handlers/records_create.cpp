#include "records_create.hpp"

#include <userver/components/component.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/postgres/result_set.hpp>

#include "../db/queries.hpp"

namespace myservice::handlers {

RecordsCreate::RecordsCreate(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value RecordsCreate::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {

  // Простая валидация входного JSON (по сути “ручной pydantic”)
  if (!request_json.HasMember("patientId") || !request_json.HasMember("doctorId") ||
      !request_json.HasMember("diagnosis") || !request_json.HasMember("notes")) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            "Missing required fields: patientId, doctorId, diagnosis, notes"});
  }

  const auto patient_id = request_json["patientId"].As<std::int64_t>();
  const auto doctor_id = request_json["doctorId"].As<std::int64_t>();
  const auto diagnosis = request_json["diagnosis"].As<std::string>();
  const auto notes = request_json["notes"].As<std::string>();

  auto res = pg_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                         myservice::db::kInsertRecord,
                         patient_id, doctor_id, diagnosis, notes);

  const auto record_id = res.AsSingleRow<std::int64_t>();

  userver::formats::json::ValueBuilder vb;
  vb["recordId"] = record_id;
  return vb.ExtractValue();
}

}  // namespace myservice::handlers