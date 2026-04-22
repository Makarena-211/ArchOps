#pragma once

#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/mongo/pool.hpp>

namespace myservice::handlers {

class MongoRecordsList final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-mongo-records-list";

  MongoRecordsList(const userver::components::ComponentConfig&,
                   const userver::components::ComponentContext&);

  userver::formats::json::Value HandleRequestJsonThrow(
      const userver::server::http::HttpRequest& request,
      const userver::formats::json::Value& request_json,
      userver::server::request::RequestContext&) const override;

 private:
  userver::storages::mongo::PoolPtr mongo_;
};

}  // namespace myservice::handlers