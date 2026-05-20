#pragma once

#include <userver/clients/http/client.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

#include "../components/auth_config.hpp"

namespace myservice::components {
class InMemoryCache;
class EventPublisherConfig;
}

namespace myservice::handlers {

class RecordsDelete final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-records-delete";

  RecordsDelete(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  userver::storages::postgres::ClusterPtr pg_;
  const myservice::components::AuthConfig& auth_cfg_;
  const myservice::components::InMemoryCache& cache_;

  userver::clients::http::Client& http_;
  const myservice::components::EventPublisherConfig& pub_cfg_;
};

}  // namespace myservice::handlers