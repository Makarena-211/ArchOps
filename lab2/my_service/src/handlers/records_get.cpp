#include "records_get.hpp"

#include <cstdint>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../components/inmemory_cache.hpp"
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

std::string CacheKeyRecord(std::int64_t record_id) {
  return "record:" + std::to_string(record_id);
}

}  // namespace

RecordsGet::RecordsGet(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      cache_(context.FindComponent<myservice::components::InMemoryCache>()) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value RecordsGet::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  const auto record_id = ParseIdPathArgOrThrow(request, "recordId");
  const auto key = CacheKeyRecord(record_id);

  if (auto cached = cache_.Get(key)) {
    return userver::formats::json::FromString(*cached);
  }

  const auto res = pg_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                                myservice::db::kGetRecordById, record_id);
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

  const auto value = out.ExtractValue();
  cache_.Put(key, userver::formats::json::ToString(value), std::chrono::seconds{30});
  return value;
}

}  // namespace myservice::handlers