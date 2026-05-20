#pragma once

#include <string>

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/schema.hpp>

namespace myservice::components {

class EventPublisherConfig final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "event-publisher-config";

  EventPublisherConfig(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  const std::string& GetUrl() const { return url_; }

 private:
  std::string url_;
};

}  // namespace myservice::components