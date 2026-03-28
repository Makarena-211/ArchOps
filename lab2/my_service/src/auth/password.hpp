#pragma once

#include <string>
#include <string_view>

namespace myservice::auth {

std::string HashPassword(std::string_view password, std::string_view salt);

}  // namespace myservice::auth