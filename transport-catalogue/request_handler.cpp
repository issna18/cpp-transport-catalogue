#include "json.h"
#include "request_handler.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>


RequestHandler::RequestHandler(const TransportCatalogue &transport_catalogue, std::ostream& out)
    : m_transport_catalogue {transport_catalogue},
      m_out {out}
{}

void RequestHandler::Read(const json::Document& jdoc) {
   ProcessRequests(jdoc.GetRoot().AsMap().at("stat_requests"));
}

json::Node RequestHandler::Print(const TransportCatalogue::BusInfo& info) const {
    using namespace std::string_literals;
    json::Dict result;
    result["request_id"] = json::Node(info.request_id);
    if (info.status == TransportCatalogue::ResultStatus::NotFound) {
        result["error_message"] = json::Node("not found"s);
        return json::Node(std::move(result));
    }
    result["curvature"] = json::Node(info.route_length/info.geo_length);
    result["route_length"] = json::Node(info.route_length);
    result["stop_count"] = json::Node(static_cast<int>(info.num_stops));
    result["unique_stop_count"] = json::Node(static_cast<int>(info.num_unique));

    return json::Node(std::move(result));
}

json::Node RequestHandler::Print(const TransportCatalogue::StopInfo& info) const {
    using namespace std::string_literals;
    json::Dict result;

    result["request_id"] = json::Node(info.request_id);
    if (info.status == TransportCatalogue::ResultStatus::NotFound) {
        result["error_message"] = json::Node("not found"s);
        return json::Node(std::move(result));
    }
    json::Array buses;
    for (const auto& bus : info.buses) {
        buses.push_back(json::Node(std::string(bus)));
    }
    result["buses"] = json::Node(std::move(buses));

    return json::Node(result);
}

void RequestHandler::ProcessRequests(const json::Node& requests){
    json::Array results;

    for (const json::Node& req : requests.AsArray()) {
        bool is_bus {"Bus" == req.AsMap().at("type").AsString()};
        int id {req.AsMap().at("id").AsInt()};
        std::string name {req.AsMap().at("name").AsString()};
        if (is_bus) {
            results.push_back(Print(m_transport_catalogue.GetInfo(BusQuery{id, name})));
        } else {
            results.push_back(Print(m_transport_catalogue.GetInfo(StopQuery{id, name})));
        }
    }

    json::PrintNode(json::Node{std::move(results)}, m_out);
    m_out << std::endl;
}

