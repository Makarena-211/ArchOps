#include "records_get.hpp"

#include <cstdint>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../db/records_queries.hpp"

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

RecordsGet::RecordsGet(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value RecordsGet::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  const auto record_id = ParseIdPathArgOrThrow(request, "recordId");

  const auto res = pg_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      myservice::db::kGetRecordById,
      record_id);

  if (res.IsEmpty()) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }

  const auto row = res.Front();
  userver::formats::json::ValueBuilder out;
  out["id"] = row["id"].As<std::int64_t>();
  out["patientId"] = row["patient_id"].As<std::int64_t>();
  out["doctorId"] = row["doctor_id"].As<std::int64_t>();
  out["diagnosis"] = row["diagnosis"].As<std::string>();
  out["notes"] = row["notes"].As<std::string>();
  out["createdAt"] = row["created_at"].As<std::string>();
  return out.ExtractValue();
}

}  // namespace myservice::handlers