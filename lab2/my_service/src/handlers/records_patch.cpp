#include "records_patch.hpp"

#include <cstdint>
#include <optional>
#include <string>

#include <userver/clients/http/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../components/event_publisher_config.hpp"
#include "../components/inmemory_cache.hpp"
#include "../db/queries.hpp"
#include "../db/records_queries.hpp"
#include "../events/event_publisher.hpp"

namespace myservice::handlers {

namespace {

std::int64_t ParseIdPathArgOrThrow(const userver::server::http::HttpRequest& request,
                                  const char* name) {
  try {
    return std::stoll(request.GetPathArg(name));
  } catch (...) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"Invalid path parameter: recordId"});
  }
}

std::string CacheKeyRecord(std::int64_t record_id) {
  return "record:" + std::to_string(record_id);
}
std::string CachePrefixPatientRecords(std::int64_t patient_id) {
  return "patient_records:" + std::to_string(patient_id) + ":";
}

}  // namespace

RecordsPatch::RecordsPatch(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      pg_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()),
      cache_(context.FindComponent<myservice::components::InMemoryCache>()),
      http_(context.FindComponent<userver::components::HttpClient>().GetHttpClient()),
      pub_cfg_(context.FindComponent<myservice::components::EventPublisherConfig>()) {}

userver::formats::json::Value RecordsPatch::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& request_json,
    userver::server::request::RequestContext&) const {
  const auto record_id = ParseIdPathArgOrThrow(request, "recordId");

  std::optional<std::string> diagnosis;
  std::optional<std::string> notes;

  if (request_json.HasMember("diagnosis")) {
    diagnosis = request_json["diagnosis"].As<std::string>();
    if (diagnosis->empty()) {
      throw userver::server::handlers::ClientError(
          userver::server::handlers::ExternalBody{"diagnosis must not be empty"});
    }
  }

  if (request_json.HasMember("notes")) {
    notes = request_json["notes"].As<std::string>();
  }

  if (!diagnosis && !notes) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            "Nothing to patch. Provide diagnosis and/or notes"});
  }

  const auto pid_res = pg_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                                    myservice::db::kGetRecordPatientIdByRecordId,
                                    record_id);
  if (pid_res.IsEmpty()) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }
  const auto patient_id = pid_res.AsSingleRow<std::int64_t>();

  const auto res = pg_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                                myservice::db::kUpdateRecordPatch,
                                record_id, diagnosis, notes);

  if (res.IsEmpty()) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }

  cache_.Invalidate(CacheKeyRecord(record_id));
  cache_.InvalidateByPrefix(CachePrefixPatientRecords(patient_id));

  userver::formats::json::ValueBuilder payload;
  payload["recordId"] = record_id;
  payload["patientId"] = patient_id;
  payload["doctorId"] = 0;
  payload["diagnosis"] = diagnosis ? *diagnosis : "";
  payload["notes"] = notes ? *notes : "";
  payload["createdAt"] = "";

  myservice::events::PublishKafkaEventViaHttp(
      http_,
      pub_cfg_.GetUrl(),
      "record:" + std::to_string(record_id),
      "MedicalRecordUpdated",
      payload.ExtractValue());

  userver::formats::json::ValueBuilder vb;
  vb["recordId"] = record_id;
  vb["patched"] = true;
  return vb.ExtractValue();
}

}  // namespace myservice::handlers