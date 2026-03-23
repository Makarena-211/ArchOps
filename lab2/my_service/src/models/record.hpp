#pragma once

#include <cstdint>
#include <string>

namespace myservice::models {

struct CreateRecordRequest {
  std::int64_t patient_id{};
  std::int64_t doctor_id{};
  std::string diagnosis;
  std::string notes;
};

}  // namespace myservice::models