#include "event_publisher.hpp"

#include <chrono>
#include <string>

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace myservice::events {

void PublishKafkaEventViaHttp(userver::clients::http::Client& http_client,
                              std::string_view base_url,
                              std::string_view key,
                              std::string_view event_type,
                              const userver::formats::json::Value& payload) {
  const std::string url = std::string(base_url) + "/publish";

  userver::formats::json::ValueBuilder body;
  body["key"] = std::string(key);
  body["eventType"] = std::string(event_type);
  body["payload"] = payload;

  auto req = http_client.CreateRequest()
                 .post(url)
                 .timeout(std::chrono::seconds{2})
                 .retry(1)
                 .data(userver::formats::json::ToString(body.ExtractValue()))
                 .headers({{"Content-Type", "application/json"}});

  auto resp = req.perform();

  const auto code = resp->status_code();

  if (code >= 300) {
  }
}

}  // namespace myservice::events