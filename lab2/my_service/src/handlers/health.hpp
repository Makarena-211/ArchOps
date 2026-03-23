#pragma once
#include <userver/server/handlers/http_handler_json_base.hpp>

namespace myservice::handlers {

class Health final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-health";

  using HttpHandlerJsonBase::HttpHandlerJsonBase;

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest&,
      const userver::formats::json::Value&,
      userver::server::request::RequestContext&) const override;
};

}  // namespace myservice::handlers