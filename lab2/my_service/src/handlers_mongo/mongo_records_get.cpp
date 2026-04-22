#include "mongo_records_get.hpp"

#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/component.hpp>

#include "../mongo/utils.hpp"

namespace myservice::handlers {

MongoRecordsGet::MongoRecordsGet(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      mongo_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()) {}

userver::formats::json::Value MongoRecordsGet::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  const auto rid = request.GetPathArg("recordId");
  const auto oid = myservice::mongo::ParseOidOrThrow(rid, "recordId");

  auto coll = mongo_->GetCollection("medical_records");
  const auto doc = coll.FindOne(userver::formats::bson::MakeDoc("_id", oid));

  if (!doc) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }

  userver::formats::json::ValueBuilder out;
  out["id"] = (*doc)["_id"].As<userver::formats::bson::Oid>().ToString();
  out["patientId"] = (*doc)["patientId"].As<userver::formats::bson::Oid>().ToString();
  out["doctorId"] = (*doc)["doctorUserId"].As<userver::formats::bson::Oid>().ToString();
  out["diagnosis"] = (*doc)["diagnosis"].As<std::string>();
  out["notes"] = (*doc)["notes"].As<std::string>();
  out["status"] = (*doc)["status"].As<std::string>();
  return out.ExtractValue();
}

}  // namespace myservice::handlers