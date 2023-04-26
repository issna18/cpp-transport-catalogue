#include "request_handler.h"

#include <string>
#include <string_view>
#include <vector>

using namespace std::string_literals;

RequestHandler::RequestHandler()

{}

void RequestHandler::ProcessBaseRequests(const json::Reader& reader) {
    std::vector<StopData> m_stops;
    std::vector<BusData> m_buses;

    const auto& requests {reader.GetBaseRequests().AsArray()};
    for (const json::Node& req : requests) {
        if (req.AsMap().at("type"s).AsString() == "Bus"s) {
            m_buses.emplace_back(json::BusDataFromJSON(req));
        } else if (req.AsMap().at("type"s).AsString() == "Stop"s) {
            m_stops.emplace_back(json::StopDataFromJSON(req));
        } else {
            throw std::invalid_argument("Invalid Request Type");
        }
    }

    for (const StopData& sd: m_stops){
        m_transport_catalogue.AddStop(sd.name, sd.coordinates);
    }
    for (const StopData& sd : m_stops) {
        for (const auto& [other, distance] : sd.adjacent){
            m_transport_catalogue.SetDistance(sd.name, other, distance);
        }
    }
    for (const BusData& bd : m_buses){
        m_transport_catalogue.AddBus(bd.first, bd.second);
    }
}

void RequestHandler::ProcessStatRequests(const json::Reader &reader, std::ostream& out){
    json::Array results;
    const auto& node {reader.GetStatRequests()};
    if (!node.IsArray()) return;
    const auto& requests {node.AsArray()};

    for (const json::Node& req : requests) {
        bool is_bus {"Bus"s == req.AsMap().at("type"s).AsString()};
        int id {req.AsMap().at("id"s).AsInt()};
        std::string name {req.AsMap().at("name"s).AsString()};
        if (is_bus) {
            results.push_back(json::ToJSON(m_transport_catalogue.GetInfo(BusQuery{id, name})));
        } else {
            results.push_back(json::ToJSON(m_transport_catalogue.GetInfo(StopQuery{id, name})));
        }
    }

    if (!results.empty()) {
        json::PrintNode(json::Node{std::move(results)}, out);
        out << std::endl;
    }
}

const std::deque<Bus>& RequestHandler::GetMap() const {
    return m_transport_catalogue.GetBuses();
}
