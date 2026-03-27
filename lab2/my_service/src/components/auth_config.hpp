#pragma once

#include <chrono>
#include <string>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/utils/strong_typedef.hpp>
#include <userver/yaml_config/schema.hpp>

namespace myservice::components {

class AuthConfig final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "auth-config";

  AuthConfig(const userver::components::ComponentConfig& config,
             const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  const std::string& GetJwtSecret() const { return jwt_secret_; }
  std::chrono::seconds GetTtl() const { return ttl_; }

 private:
  std::string jwt_secret_;
  std::chrono::seconds ttl_{3600};
};

}  // namespace myservice::components