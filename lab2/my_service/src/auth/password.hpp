#pragma once

#include <string>
#include <string_view>

namespace myservice::auth {

// Для лабораторной: sha256(secret_salt + password)
// (salt возьмём из jwt-secret, чтобы не вводить ещё один секрет)
std::string HashPassword(std::string_view password, std::string_view salt);

}  // namespace myservice::auth