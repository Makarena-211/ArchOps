#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>

#include "../components/auth_config.hpp"

namespace myservice::handlers {

class AuthRegister final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-auth-register";

  AuthRegister(const userver::components::ComponentConfig&,
               const userver::components::ComponentContext&);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest&,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext&) const override;

 private:
  userver::storages::postgres::ClusterPtr pg_;
  const myservice::components::AuthConfig& cfg_;
};

}  // namespace myservice::handlers