#include "component_list.hpp"

#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>

#include "auth_config.hpp"
#include "event_publisher_config.hpp"
#include "inmemory_cache.hpp"
#include "rate_limiter.hpp"

#include "../handlers/auth_login.hpp"
#include "../handlers/auth_register.hpp"
#include "../handlers/health.hpp"

#include "../handlers/records_create.hpp"
#include "../handlers/records_delete.hpp"
#include "../handlers/records_list.hpp"
#include "../handlers/records_patch.hpp"
#include "../handlers/records_put.hpp"
#include "../handlers/records_get.hpp"

#include "../handlers/patients_create.hpp"
#include "../handlers/patients_search.hpp"
#include "../handlers/patients_records_create.hpp"
#include "../handlers/patient_records_list.hpp"

#include "../handlers/read_records_get.hpp"

#include "../handlers/users_get_by_login.hpp"
#include "../handlers/users_search_by_name.hpp"

// ===== Mongo handlers =====
#include "../handlers_mongo/mongo_records_create.hpp"
#include "../handlers_mongo/mongo_records_list.hpp"
#include "../handlers_mongo/mongo_records_get.hpp"
#include "../handlers_mongo/mongo_records_put.hpp"
#include "../handlers_mongo/mongo_records_patch.hpp"
#include "../handlers_mongo/mongo_records_delete.hpp"

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
      .Append<userver::components::HttpClient>()

      .Append<myservice::components::AuthConfig>()
      .Append<myservice::components::InMemoryCache>()
      .Append<myservice::components::RateLimiter>()
      .Append<myservice::components::EventPublisherConfig>()

      .Append<myservice::handlers::AuthRegister>()
      .Append<myservice::handlers::AuthLogin>()

      .Append<myservice::handlers::Health>()

      .Append<myservice::handlers::RecordsPut>()
      .Append<myservice::handlers::RecordsPatch>()
      .Append<myservice::handlers::RecordsDelete>()
      .Append<myservice::handlers::RecordsCreate>()
      .Append<myservice::handlers::RecordsList>()
      .Append<myservice::handlers::RecordsGet>()

      .Append<myservice::handlers::PatientsCreate>()
      .Append<myservice::handlers::PatientsSearch>()
      .Append<myservice::handlers::PatientRecordsCreate>()
      .Append<myservice::handlers::PatientRecordsList>()

      .Append<myservice::handlers::ReadRecordsGet>()

      .Append<myservice::handlers::UsersGetByLogin>()
      .Append<myservice::handlers::UsersSearchByName>()

      // ===== Mongo CRUD =====
      .Append<myservice::handlers::MongoRecordsCreate>()
      .Append<myservice::handlers::MongoRecordsList>()
      .Append<myservice::handlers::MongoRecordsGet>()
      .Append<myservice::handlers::MongoRecordsPut>()
      .Append<myservice::handlers::MongoRecordsPatch>()
      .Append<myservice::handlers::MongoRecordsDelete>();
}

}  // namespace myservice::components