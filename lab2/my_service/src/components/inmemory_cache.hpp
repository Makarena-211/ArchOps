#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/engine/mutex.hpp>
#include <userver/yaml_config/schema.hpp>

namespace myservice::components {

class InMemoryCache final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "inmemory-cache";

  InMemoryCache(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  std::optional<std::string> Get(std::string_view key) const;

  // MUST be const: handlers keep cache_ as const ref and HandleRequest... is const
  void Put(std::string key, std::string value, std::chrono::seconds ttl) const;

  void Invalidate(std::string_view key) const;
  void InvalidateByPrefix(std::string_view prefix) const;

  std::uint64_t GetHits() const;
  std::uint64_t GetMisses() const;

 private:
  struct Entry {
    std::string value;
    std::chrono::steady_clock::time_point expires_at;
  };

  void CleanupExpiredUnlocked(std::chrono::steady_clock::time_point now) const;

  std::size_t max_entries_{5000};

  mutable userver::engine::Mutex mutex_;
  mutable std::unordered_map<std::string, Entry> map_;

  mutable std::uint64_t hits_{0};
  mutable std::uint64_t misses_{0};
};

}  // namespace myservice::components