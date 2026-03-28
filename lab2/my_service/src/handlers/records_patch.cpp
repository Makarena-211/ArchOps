#include "records_patch.hpp"

#include <cstdint>
#include <optional>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../db/queries.hpp"

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

}  // namespace

RecordsPatch::RecordsPatch(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

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

  const auto res = pg_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                                myservice::db::kUpdateRecordPatch,
                                record_id, diagnosis, notes);

  if (res.IsEmpty()) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }

  userver::formats::json::ValueBuilder vb;
  vb["recordId"] = record_id;
  vb["patched"] = true;
  return vb.ExtractValue();
}

}  // namespace myservice::handlers