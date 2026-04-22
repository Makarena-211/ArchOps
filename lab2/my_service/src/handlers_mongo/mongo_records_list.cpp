#include "mongo_records_list.hpp"

#include <cstdint>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/bson/types.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/mongo/collection.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/options.hpp>

#include "../mongo/utils.hpp"

namespace myservice::handlers {

MongoRecordsList::MongoRecordsList(const userver::components::ComponentConfig& config,
                                   const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      mongo_(context.FindComponent<userver::components::Mongo>("mongo-db").GetPool()) {}

userver::formats::json::Value MongoRecordsList::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {

  std::int64_t limit = myservice::mongo::GetIntArgOr(request, "limit", 50);
  std::int64_t offset = myservice::mongo::GetIntArgOr(request, "offset", 0);
  limit = myservice::mongo::ClampLimit(limit);
  offset = myservice::mongo::ClampOffset(offset);

  auto coll = mongo_->GetCollection("medical_records");

  const auto filter = userver::formats::bson::MakeDoc();
  const auto total = coll.Count(filter);

  const userver::storages::mongo::options::Sort sort{
      {{"createdAt", userver::storages::mongo::options::Sort::Direction::kDescending}}};

  auto cursor = coll.Find(
      filter,
      userver::storages::mongo::options::Skip{static_cast<std::size_t>(offset)},
      userver::storages::mongo::options::Limit{static_cast<std::size_t>(limit)},
      sort);

  userver::formats::json::ValueBuilder items(userver::formats::json::Type::kArray);
  for (const auto& d : cursor) {
    userver::formats::json::ValueBuilder it;
    it["id"] = d["_id"].As<userver::formats::bson::Oid>().ToString();
    it["patientId"] = d["patientId"].As<userver::formats::bson::Oid>().ToString();
    it["doctorId"] = d["doctorUserId"].As<userver::formats::bson::Oid>().ToString();
    it["diagnosis"] = d["diagnosis"].As<std::string>();
    it["notes"] = d["notes"].As<std::string>();
    items.PushBack(it.ExtractValue());
  }

  userver::formats::json::ValueBuilder out;
  out["items"] = items.ExtractValue();
  out["limit"] = limit;
  out["offset"] = offset;
  out["total"] = static_cast<std::int64_t>(total);
  return out.ExtractValue();
}

}  // namespace myservice::handlers