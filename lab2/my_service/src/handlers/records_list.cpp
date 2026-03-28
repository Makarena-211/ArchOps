#include "records_list.hpp"

#include <algorithm>
#include <cstdint>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/datetime.hpp>

#include "../db/queries.hpp"

namespace myservice::handlers {

namespace {

std::int64_t GetIntArgOr(const userver::server::http::HttpRequest& request,
                         const std::string& name,
                         std::int64_t def) {
  const auto value = request.GetArg(name);
  if (value.empty()) return def;
  try {
    return std::stoll(value);
  } catch (...) {
    return def;
  }
}

}  // namespace

RecordsList::RecordsList(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value RecordsList::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  // GET /api/records?limit=50&offset=0
  std::int64_t limit = GetIntArgOr(request, "limit", 50);
  std::int64_t offset = GetIntArgOr(request, "offset", 0);

  limit = std::clamp<std::int64_t>(limit, 1, 200);
  offset = std::max<std::int64_t>(offset, 0);

  // total
  const auto total_res = pg_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      myservice::db::kCountAllRecords);
  const auto total = total_res.AsSingleRow<std::int64_t>();

  // items
  const auto res = pg_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      myservice::db::kSelectAllRecords,
      limit, offset);

  userver::formats::json::ValueBuilder items(userver::formats::json::Type::kArray);

  for (const auto& row : res) {
    userver::formats::json::ValueBuilder item;
    item["id"] = row["id"].As<std::int64_t>();
    item["patientId"] = row["patient_id"].As<std::int64_t>();
    item["doctorId"] = row["doctor_id"].As<std::int64_t>();
    item["diagnosis"] = row["diagnosis"].As<std::string>();
    item["notes"] = row["notes"].As<std::string>();
    item["createdAt"] = row["created_at"].As<std::string>();

    items.PushBack(item.ExtractValue());
  }

  userver::formats::json::ValueBuilder out;
  out["items"] = items.ExtractValue();
  out["limit"] = limit;
  out["offset"] = offset;
  out["total"] = total;

  return out.ExtractValue();
}

}  // namespace myservice::handlers