#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

namespace myservice::components {
class InMemoryCache;
}

namespace myservice::handlers {

class RecordsPut final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-records-put";

  RecordsPut(const userver::components::ComponentConfig& config,
             const userver::components::ComponentContext& context);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext& context) const override;

 private:
  userver::storages::postgres::ClusterPtr pg_;
  const myservice::components::InMemoryCache& cache_;
};

}  // namespace myservice::handlers