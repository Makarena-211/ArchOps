#include "mongo_records_put.hpp"

#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/bson/value_builder.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/utils/datetime.hpp>

#include "../mongo/utils.hpp"

namespace myservice::handlers {

MongoRecordsPut::MongoRecordsPut(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      mongo_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()) {}

userver::formats::json::Value MongoRecordsPut::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext&) const {

  const auto rid = request.GetPathArg("recordId");
  const auto oid = myservice::mongo::ParseOidOrThrow(rid, "recordId");

  myservice::mongo::RequireFieldsOrThrow(json, {"diagnosis", "notes"});
  const auto diagnosis = json["diagnosis"].As<std::string>();
  const auto notes = json["notes"].As<std::string>();

  if (diagnosis.empty()) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"diagnosis must not be empty"});
  }

  userver::formats::bson::ValueBuilder set_doc;
  set_doc["diagnosis"] = diagnosis;
  set_doc["notes"] = notes;
  set_doc["updatedAt"] = userver::utils::datetime::Now();

  userver::formats::bson::ValueBuilder update;
  update["$set"] = set_doc.ExtractValue();

  auto coll = mongo_->GetCollection("medical_records");
  const auto res = coll.UpdateOne(
      userver::formats::bson::MakeDoc("_id", oid),
      update.ExtractValue());

  if (res.MatchedCount() == 0) {
    throw userver::server::handlers::ResourceNotFound(
        userver::server::handlers::ExternalBody{"Record not found"});
  }

  userver::formats::json::ValueBuilder out;
  out["recordId"] = rid;
  out["updated"] = true;
  return out.ExtractValue();
}

}  // namespace myservice::handlers