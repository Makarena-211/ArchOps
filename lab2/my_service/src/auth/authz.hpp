#pragma once

#include <string>
#include <string_view>

#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/http/http_request.hpp>

#include "../components/auth_config.hpp"
#include "bearer.hpp"
#include "jwt.hpp"

namespace myservice::auth {

inline JwtClaims VerifyRequestAndGetClaimsOrThrow(
    const userver::server::http::HttpRequest& request,
    const myservice::components::AuthConfig& cfg) {
  const std::string token = ExtractBearerTokenOrThrow(request);

  JwtClaims claims;
  if (!VerifyTokenHS256(std::string_view{token}, cfg.GetJwtSecret(), claims)) {
    throw userver::server::handlers::Unauthorized(
        userver::server::handlers::ExternalBody{"Invalid or expired token"});
  }
  return claims;
}

inline void RequireRoleOrThrow(
    std::string_view role, std::initializer_list<std::string_view> allowed) {
  for (const auto a : allowed) {
    if (role == a) return;
  }
  throw userver::server::handlers::ExceptionWithCode<
      userver::server::handlers::HandlerErrorCode::kForbidden>(
      userver::server::handlers::ExternalBody{"Insufficient permissions"});
}
}  // namespace myservice::auth