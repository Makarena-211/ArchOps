#include "auth_config.hpp"

#include <userver/yaml_config/merge_schemas.hpp>

namespace myservice::components {

AuthConfig::AuthConfig(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  jwt_secret_ = config["jwt-secret"].As<std::string>();
  ttl_ = std::chrono::seconds{config["token-ttl-seconds"].As<int>(3600)};
}

userver::yaml_config::Schema AuthConfig::GetStaticConfigSchema() {
  // Важно: схема должна описывать поля, которые читает constructor
  return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: Auth/JWT settings
additionalProperties: false
properties:
  jwt-secret:
    type: string
    description: Secret key used for HS256 signing and also as password salt (lab simplification)
  token-ttl-seconds:
    type: integer
    description: Access token lifetime in seconds
    default: 3600
)");
}

}  // namespace myservice::components