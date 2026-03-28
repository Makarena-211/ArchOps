#include "password.hpp"

#include <userver/crypto/hash.hpp>

namespace myservice::auth {

std::string HashPassword(std::string_view password, std::string_view salt) {
  std::string data;
  data.reserve(salt.size() + password.size());
  data.append(salt);
  data.append(password);

  return userver::crypto::hash::Sha256(data);
}

}  // namespace myservice::auth