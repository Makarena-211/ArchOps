#pragma once

#include <string_view>

#include <userver/clients/http/client.hpp>
#include <userver/formats/json/value.hpp>

namespace myservice::events {

void PublishKafkaEventViaHttp(userver::clients::http::Client& http_client,
                              std::string_view base_url,
                              std::string_view key,
                              std::string_view event_type,
                              const userver::formats::json::Value& payload);

}  // namespace myservice::events