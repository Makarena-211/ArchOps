#include "patients_search.hpp"

#include <algorithm>
#include <cstdint>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../db/patient_queries.hpp"

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

PatientsSearch::PatientsSearch(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context) {
  pg_ = context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster();
}

userver::formats::json::Value PatientsSearch::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  // GET /api/patients/search?mask=иван&limit=50&offset=0
  const auto mask_raw = request.GetArg("mask");
  if (mask_raw.empty()) {
    userver::formats::json::ValueBuilder vb;
    vb["items"] = userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray).ExtractValue();
    vb["limit"] = 50;
    vb["offset"] = 0;
    vb["total"] = 0;
    return vb.ExtractValue();
  }

  std::int64_t limit = GetIntArgOr(request, "limit", 50);
  std::int64_t offset = GetIntArgOr(request, "offset", 0);
  limit = std::clamp<std::int64_t>(limit, 1, 200);
  offset = std::max<std::int64_t>(offset, 0);

  const std::string mask = "%" + mask_raw + "%";

  const auto res = pg_->Execute(
      userver::storages::postgres::ClusterHostType::kSlave,
      myservice::db::kSearchPatientsByMask,
      mask, limit, offset);

  userver::formats::json::ValueBuilder items(userver::formats::json::Type::kArray);
  for (const auto& row : res) {
    userver::formats::json::ValueBuilder it;
    it["patientId"] = row["patient_id"].As<std::int64_t>();
    it["userId"] = row["user_id"].As<std::int64_t>();
    it["email"] = row["email"].As<std::string>();
    it["firstName"] = row["first_name"].As<std::string>();
    it["lastName"] = row["last_name"].As<std::string>();
    it["middleName"] = row["middle_name"].IsNull() ? "" : row["middle_name"].As<std::string>();
    it["birthDate"] = row["birth_date"].IsNull() ? "" : row["birth_date"].As<std::string>();
    items.PushBack(it.ExtractValue());
  }

  userver::formats::json::ValueBuilder out;
  out["items"] = items.ExtractValue();
  out["limit"] = limit;
  out["offset"] = offset;
  out["total"] = static_cast<std::int64_t>(res.Size());  // просто размер страницы
  return out.ExtractValue();
}

}  // namespace myservice::handlers