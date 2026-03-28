#include "component_list.hpp"

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>

#include "auth_config.hpp"

#include "../handlers/auth_login.hpp"
#include "../handlers/auth_register.hpp"
#include "../handlers/health.hpp"
#include "../handlers/records_create.hpp"
#include "../handlers/records_delete.hpp"
#include "../handlers/records_list.hpp"
#include "../handlers/records_patch.hpp"
#include "../handlers/records_put.hpp"

namespace myservice::components {

userver::components::ComponentList MakeComponentList() {
  return userver::components::MinimalServerComponentList()
      .Append<userver::server::handlers::Ping>()
      .Append<userver::components::TestsuiteSupport>()
      .AppendComponentList(userver::clients::http::ComponentList())
      .Append<userver::clients::dns::Component>()
      .Append<userver::server::handlers::TestsControl>()
      .Append<userver::congestion_control::Component>()
      .Append<userver::components::Postgres>("postgres-db")

      // ваш auth config + auth handlers
      .Append<myservice::components::AuthConfig>()
      .Append<myservice::handlers::AuthRegister>()
      .Append<myservice::handlers::AuthLogin>()

      // ваши API handlers
      .Append<myservice::handlers::Health>()
      .Append<myservice::handlers::RecordsPut>()
      .Append<myservice::handlers::RecordsPatch>()
      .Append<myservice::handlers::RecordsDelete>()
      .Append<myservice::handlers::RecordsCreate>()
      .Append<myservice::handlers::RecordsList>();
}

}  // namespace myservice::components