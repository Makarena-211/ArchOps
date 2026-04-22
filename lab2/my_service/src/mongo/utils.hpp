#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

#include <userver/formats/bson/types.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_request.hpp>

namespace myservice::mongo {

inline userver::formats::bson::Oid ParseOidOrThrow(std::string_view s,
                                                   std::string_view field_name = "id") {
  try {
    return userver::formats::bson::Oid{std::string{s}};
  } catch (...) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{
            std::string{"Invalid "} + std::string(field_name) +
            " (expected Mongo ObjectId hex string)"});  // 400
  }
}

inline std::int64_t GetIntArgOr(const userver::server::http::HttpRequest& request,
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

inline void RequireFieldsOrThrow(const userver::formats::json::Value& json,
                                 std::initializer_list<const char*> fields) {
  for (const auto f : fields) {
    if (!json.HasMember(f)) {
      throw userver::server::handlers::ClientError(
          userver::server::handlers::ExternalBody{std::string{"Missing required field: "} + f});
    }
  }
}

inline std::int64_t ClampLimit(std::int64_t limit) {
  return std::clamp<std::int64_t>(limit, 1, 200);
}

inline std::int64_t ClampOffset(std::int64_t offset) {
  return std::max<std::int64_t>(offset, 0);
}

}  // namespace myservice::mongo