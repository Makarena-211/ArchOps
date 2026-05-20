#include "patients_search.hpp"

#include <algorithm>
#include <cstdint>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/postgres/component.hpp>

#include <userver/http/status_code.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "../components/rate_limiter.hpp"
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

std::string GetClientKey(const userver::server::http::HttpRequest& request) {
  const auto cid = request.GetHeader("X-Client-Id");
  if (!cid.empty()) return cid;

  const auto xff = request.GetHeader("X-Forwarded-For");
  if (!xff.empty()) return xff;

  const auto xrip = request.GetHeader("X-Real-IP");
  if (!xrip.empty()) return xrip;

  return "unknown";
}

void ApplyRateHeaders(userver::server::http::HttpResponse& resp,
                      const myservice::components::RateLimitDecision& d) {
  resp.SetHeader(std::string_view{"X-RateLimit-Limit"}, std::to_string(d.limit));
  resp.SetHeader(std::string_view{"X-RateLimit-Remaining"}, std::to_string(d.remaining));
  resp.SetHeader(std::string_view{"X-RateLimit-Reset"}, std::to_string(d.reset_seconds));
}

}  // namespace

PatientsSearch::PatientsSearch(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      pg_(context.FindComponent<userver::components::Postgres>("postgres-db").GetCluster()),
      rl_(context.FindComponent<myservice::components::RateLimiter>()) {
  rate_limit_name_ = config["rate-limit-name"].As<std::string>("");
}

userver::yaml_config::Schema PatientsSearch::GetStaticConfigSchema() {
  // Добавляем поддержку rate-limit-name в схему handler'а
  return userver::yaml_config::MergeSchemas<userver::server::handlers::HttpHandlerJsonBase>(
      R"(
type: object
description: Patients search handler (optionally rate limited)
additionalProperties: false
properties:
  rate-limit-name:
    type: string
    description: Rate limiter bucket group name (empty disables rate limiting)
    default: ""
)");
}

userver::formats::json::Value PatientsSearch::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  if (!rate_limit_name_.empty()) {
    const auto decision = rl_.Check(rate_limit_name_, GetClientKey(request));
    ApplyRateHeaders(request.GetHttpResponse(), decision);

    if (!decision.allowed) {
      request.GetHttpResponse().SetStatus(userver::http::StatusCode::kTooManyRequests);

      userver::formats::json::ValueBuilder err;
      err["message"] = "Too Many Requests";
      return err.ExtractValue();
    }
  }

  const auto mask_raw = request.GetArg("mask");
  if (mask_raw.empty()) {
    userver::formats::json::ValueBuilder vb;
    vb["items"] =
        userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray).ExtractValue();
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

  const auto res = pg_->Execute(userver::storages::postgres::ClusterHostType::kSlave,
                                myservice::db::kSearchPatientsByMask, mask, limit, offset);

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
  out["total"] = static_cast<std::int64_t>(res.Size());
  return out.ExtractValue();
}

}  // namespace myservice::handlers