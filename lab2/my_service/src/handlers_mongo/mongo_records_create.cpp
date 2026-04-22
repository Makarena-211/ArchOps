#include "mongo_records_create.hpp"

#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/bson/value_builder.hpp>
#include <userver/formats/common/type.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/utils/datetime.hpp>

#include "../auth/authz.hpp"
#include "../mongo/utils.hpp"

namespace myservice::handlers {

MongoRecordsCreate::MongoRecordsCreate(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      mongo_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()),
      auth_cfg_(context.FindComponent<myservice::components::AuthConfig>()) {}

userver::formats::json::Value MongoRecordsCreate::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& json,
    userver::server::request::RequestContext&) const {

  const auto claims = myservice::auth::VerifyRequestAndGetClaimsOrThrow(request, auth_cfg_);
  myservice::auth::RequireRoleOrThrow(claims.role, {"doctor", "admin"});

  myservice::mongo::RequireFieldsOrThrow(json, {"patientId", "doctorId", "diagnosis", "notes"});

  const auto patient_oid =
      myservice::mongo::ParseOidOrThrow(json["patientId"].As<std::string>(), "patientId");
  const auto doctor_oid =
      myservice::mongo::ParseOidOrThrow(json["doctorId"].As<std::string>(), "doctorId");

  const auto diagnosis = json["diagnosis"].As<std::string>();
  const auto notes = json["notes"].As<std::string>();

  if (diagnosis.empty() || notes.empty()) {
    throw userver::server::handlers::ClientError(
        userver::server::handlers::ExternalBody{"diagnosis/notes must not be empty"});
  }

  auto coll = mongo_->GetCollection("medical_records");

  const userver::formats::bson::Oid record_id{};

  userver::formats::bson::ValueBuilder doc;
  doc["_id"] = record_id;
  doc["patientId"] = patient_oid;
  doc["doctorUserId"] = doctor_oid;
  doc["diagnosis"] = diagnosis;
  doc["notes"] = notes;
  doc["status"] = "final";

  doc["tags"] = userver::formats::bson::ValueBuilder(userver::formats::common::Type::kArray)
                    .ExtractValue();
  doc["attachments"] = userver::formats::bson::ValueBuilder(userver::formats::common::Type::kArray)
                           .ExtractValue();

  const auto now = userver::utils::datetime::Now();
  doc["createdAt"] = now;
  doc["updatedAt"] = now;

  coll.InsertOne(doc.ExtractValue());

  userver::formats::json::ValueBuilder out;
  out["recordId"] = record_id.ToString();
  return out.ExtractValue();
}

}  // namespace myservice::handlers