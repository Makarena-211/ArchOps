#include "records_delete.hpp"

#include <cstdint>

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../auth/authz.hpp"
#include "../components/inmemory_cache.hpp"
#include "../db/queries.hpp"
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
std::string CachePrefixPatientRecords(std::int64_t patient_id) {
  return "patient_records:" + std::to_string(patient_id) + ":";
}

}  // namespace

RecordsDelete::RecordsDelete(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      auth_cfg_(context.FindComponent<myservice::components::AuthConfig>()),
      cache_(context.FindComponent<myservice::components::InMemoryCache>()) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value RecordsDelete::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  const auto claims = myservice::auth::VerifyRequestAndGetClaimsOrThrow(request, auth_cfg_);
  myservice::auth::RequireRoleOrThrow(claims.role, {"admin"});

  const auto record_id = ParseIdPathArgOrThrow(request, "recordId");

  const auto pid_res = pg_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                                    myservice::db::kGetRecordPatientIdByRecordId,
                                    record_id);
  if (pid_res.IsEmpty()) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }
  const auto patient_id = pid_res.AsSingleRow<std::int64_t>();

  const auto res = pg_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                                myservice::db::kDeleteRecord, record_id);

  if (res.IsEmpty()) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }

  cache_.Invalidate(CacheKeyRecord(record_id));
  cache_.InvalidateByPrefix(CachePrefixPatientRecords(patient_id));

  userver::formats::json::ValueBuilder vb;
  vb["recordId"] = record_id;
  vb["deleted"] = true;
  return vb.ExtractValue();
}

}  // namespace myservice::handlers