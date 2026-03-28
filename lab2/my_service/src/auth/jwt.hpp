#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace myservice::auth {

struct JwtClaims {
  std::int64_t user_id{};
  std::string email;
  std::string role;
  std::int64_t exp{};
};

std::string CreateTokenHS256(const JwtClaims& claims, std::string_view secret);

bool VerifyTokenHS256(std::string_view token, std::string_view secret, JwtClaims& out);

}  // namespace myservice::auth