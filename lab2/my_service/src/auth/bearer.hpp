#pragma once

#include <string>
#include <string_view>

#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_request.hpp>

namespace myservice::auth {

inline std::string ExtractBearerTokenOrThrow(const userver::server::http::HttpRequest& request) {
  const std::string auth = request.GetHeader("Authorization");

  constexpr std::string_view kPrefix = "Bearer ";
  if (auth.size() <= kPrefix.size() ||
      std::string_view{auth}.substr(0, kPrefix.size()) != kPrefix) {
    throw userver::server::handlers::Unauthorized(
        userver::server::handlers::ExternalBody{"Missing/invalid Authorization: Bearer token"});
  }

  return auth.substr(kPrefix.size());
}

}  // namespace myservice::auth