#include "rate_limiter.hpp"

#include <algorithm>
#include <cmath>

#include <userver/yaml_config/merge_schemas.hpp>

namespace myservice::components {

RateLimiter::RateLimiter(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  def_.capacity = config["default"]["capacity"].As<std::int64_t>(60);
  def_.refill_per_second = config["default"]["refill-per-second"].As<double>(1.0);
}

userver::yaml_config::Schema RateLimiter::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(
      R"(
type: object
description: Simple in-process token bucket rate limiter
additionalProperties: false
properties:
  default:
    type: object
    description: Default token bucket params
    additionalProperties: false
    properties:
      capacity:
        type: integer
        description: Max tokens in bucket
        default: 60
      refill-per-second:
        type: number
        description: Token refill rate per second
        default: 1
)");
}

RateLimitDecision RateLimiter::Check(std::string_view endpoint_name, std::string_view key) const {
  (void)endpoint_name;

  const auto now = std::chrono::steady_clock::now();
  const std::string bucket_key = std::string(key);

  std::lock_guard lock(mutex_);
  auto& b = buckets_[bucket_key];

  if (b.last.time_since_epoch().count() == 0) {
    b.tokens = static_cast<double>(def_.capacity);
    b.last = now;
  }

  const std::chrono::duration<double> dt = now - b.last;
  b.last = now;

  b.tokens = std::min<double>(static_cast<double>(def_.capacity),
                              b.tokens + dt.count() * def_.refill_per_second);

  RateLimitDecision d;
  d.limit = def_.capacity;

  if (b.tokens >= 1.0) {
    b.tokens -= 1.0;
    d.allowed = true;
    d.remaining = static_cast<std::int64_t>(std::floor(b.tokens));
    d.reset_seconds = 0;
    return d;
  }

  d.allowed = false;
  d.remaining = 0;

  if (def_.refill_per_second > 0.0) {
    const double missing = 1.0 - b.tokens;
    d.reset_seconds = static_cast<std::int64_t>(std::ceil(missing / def_.refill_per_second));
  } else {
    d.reset_seconds = 60;
  }
  return d;
}

}  // namespace myservice::components