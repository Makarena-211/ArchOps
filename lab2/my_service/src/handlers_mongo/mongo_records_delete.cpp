#include "mongo_records_delete.hpp"

#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/bson/value_builder.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/component.hpp>

#include "../auth/authz.hpp"
#include "../mongo/utils.hpp"

namespace myservice::handlers {

MongoRecordsDelete::MongoRecordsDelete(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      mongo_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()),
      auth_cfg_(context.FindComponent<myservice::components::AuthConfig>()) {}

userver::formats::json::Value MongoRecordsDelete::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  const auto claims = myservice::auth::VerifyRequestAndGetClaimsOrThrow(request, auth_cfg_);
  myservice::auth::RequireRoleOrThrow(claims.role, {"admin"});

  const auto rid = request.GetPathArg("recordId");
  const auto oid = myservice::mongo::ParseOidOrThrow(rid, "recordId");

  auto coll = mongo_->GetCollection("medical_records");
  const auto res = coll.DeleteOne(userver::formats::bson::MakeDoc("_id", oid));

  if (res.DeletedCount() == 0) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }

  userver::formats::json::ValueBuilder out;
  out["recordId"] = rid;
  out["deleted"] = true;
  return out.ExtractValue();
}

}  // namespace myservice::handlers