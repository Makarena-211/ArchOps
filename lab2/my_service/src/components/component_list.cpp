#include "component_list.hpp"

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/storages/postgres/component.hpp>

#include "../handlers/health.hpp"
#include "../handlers/records_create.hpp"

namespace myservice::components {

userver::components::ComponentList MakeComponentList() {
  return userver::components::MinimalServerComponentList()
      // То, что ожидается static_config.yaml и testsuite:
      .Append<userver::server::handlers::Ping>()                         // handler-ping
      .Append<userver::components::TestsuiteSupport>()                   // testsuite-support
      .AppendComponentList(userver::clients::http::ComponentList())      // http-client/http-client-core
      .Append<userver::clients::dns::Component>()                        // dns-client
      .Append<userver::server::handlers::TestsControl>()                 // tests-control
      .Append<userver::congestion_control::Component>()                  // congestion-control

      // Ваше:
      .Append<userver::components::Postgres>("postgres-db")

      .Append<myservice::handlers::Health>()
      .Append<myservice::handlers::RecordsCreate>();
}

}  // namespace myservice::components