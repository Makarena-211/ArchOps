#include "health.hpp"
#include <userver/formats/json/value_builder.hpp>

namespace myservice::handlers {

userver::formats::json::Value Health::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest&,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
  userver::formats::json::ValueBuilder vb;
  vb["status"] = "ok";
  return vb.ExtractValue();
}

}  // namespace myservice::handlers