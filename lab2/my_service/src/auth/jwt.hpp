#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace myservice::auth {

struct JwtClaims {
  std::int64_t user_id{};
  std::string email;
  std::string role;
  std::int64_t exp{}; // unix seconds
};

std::string CreateTokenHS256(const JwtClaims& claims, std::string_view secret);

// Возвращает true/false; если true — claims заполнен
bool VerifyTokenHS256(std::string_view token, std::string_view secret, JwtClaims& out);

}  // namespace myservice::auth