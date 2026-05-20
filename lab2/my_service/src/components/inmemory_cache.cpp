#include "inmemory_cache.hpp"

#include <userver/yaml_config/merge_schemas.hpp>

namespace myservice::components {

InMemoryCache::InMemoryCache(const userver::components::ComponentConfig& config,
                             const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  max_entries_ = config["max-entries"].As<std::size_t>(5000);
}

userver::yaml_config::Schema InMemoryCache::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(
      R"(
type: object
description: Simple in-process cache with TTL (no Redis)
additionalProperties: false
properties:
  max-entries:
    type: integer
    description: Maximum number of cached entries (best-effort)
    default: 5000
)");
}

void InMemoryCache::CleanupExpiredUnlocked(std::chrono::steady_clock::time_point now) const {
  for (auto it = map_.begin(); it != map_.end();) {
    if (it->second.expires_at <= now) it = map_.erase(it);
    else ++it;
  }
}

std::optional<std::string> InMemoryCache::Get(std::string_view key) const {
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard lock(mutex_);

  CleanupExpiredUnlocked(now);

  auto it = map_.find(std::string(key));
  if (it == map_.end()) {
    ++misses_;
    return std::nullopt;
  }
  if (it->second.expires_at <= now) {
    map_.erase(it);
    ++misses_;
    return std::nullopt;
  }

  ++hits_;
  return it->second.value;
}

void InMemoryCache::Put(std::string key, std::string value, std::chrono::seconds ttl) const {
  const auto now = std::chrono::steady_clock::now();
  std::lock_guard lock(mutex_);

  CleanupExpiredUnlocked(now);

  while (map_.size() >= max_entries_ && !map_.empty()) {
    map_.erase(map_.begin());
  }
  map_[std::move(key)] = Entry{std::move(value), now + ttl};
}

void InMemoryCache::Invalidate(std::string_view key) const {
  std::lock_guard lock(mutex_);
  map_.erase(std::string(key));
}

void InMemoryCache::InvalidateByPrefix(std::string_view prefix) const {
  std::lock_guard lock(mutex_);
  for (auto it = map_.begin(); it != map_.end();) {
    if (it->first.rfind(prefix, 0) == 0) it = map_.erase(it);
    else ++it;
  }
}

std::uint64_t InMemoryCache::GetHits() const {
  std::lock_guard lock(mutex_);
  return hits_;
}

std::uint64_t InMemoryCache::GetMisses() const {
  std::lock_guard lock(mutex_);
  return misses_;
}

}  // namespace myservice::components