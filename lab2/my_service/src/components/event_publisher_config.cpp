#include "event_publisher_config.hpp"

#include <userver/yaml_config/merge_schemas.hpp>

namespace myservice::components {

EventPublisherConfig::EventPublisherConfig(const userver::components::ComponentConfig& config,
                                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context) {
  url_ = config["url"].As<std::string>();
}

userver::yaml_config::Schema EventPublisherConfig::GetStaticConfigSchema() {
  return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: Domain events publisher settings (HTTP gateway)
additionalProperties: false
properties:
  url:
    type: string
    description: Base URL of HTTP Kafka gateway (e.g. http://event-gateway:8090)
)");
}

}  // namespace myservice::components