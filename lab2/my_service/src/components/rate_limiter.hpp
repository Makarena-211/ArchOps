#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/engine/mutex.hpp>
#include <userver/yaml_config/schema.hpp>

namespace myservice::components {

struct RateLimitDecision {
  bool allowed{true};
  std::int64_t limit{0};
  std::int64_t remaining{0};
  std::int64_t reset_seconds{0};
};

class RateLimiter final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "rate-limiter";

  RateLimiter(const userver::components::ComponentConfig& config,
              const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  RateLimitDecision Check(std::string_view endpoint_name, std::string_view key) const;

 private:
  struct EndpointConfig {
    std::int64_t capacity{60};
    double refill_per_second{1.0};
  };

  struct Bucket {
    double tokens{0.0};
    std::chrono::steady_clock::time_point last{};
  };

  mutable userver::engine::Mutex mutex_;
  EndpointConfig def_{};
  mutable std::unordered_map<std::string, Bucket> buckets_;
};

}  // namespace myservice::components