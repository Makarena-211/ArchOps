#pragma once

#include <string>

#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/yaml_config/schema.hpp>

namespace myservice::components {
class RateLimiter;
}

namespace myservice::handlers {

class PatientsSearch final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-patients-search";

  PatientsSearch(const userver::components::ComponentConfig&,
                 const userver::components::ComponentContext&);

  static userver::yaml_config::Schema GetStaticConfigSchema();

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  userver::storages::postgres::ClusterPtr pg_;

  const myservice::components::RateLimiter& rl_;
  std::string rate_limit_name_;  // empty => disabled
};

}  // namespace myservice::handlers