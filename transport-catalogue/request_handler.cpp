#include "request_handler.h"
#include "map_renderer.h"

#include <string>
#include <sstream>
#include <string_view>
#include <vector>

using namespace std::string_literals;
using namespace std::string_view_literals;

RequestHandler::RequestHandler()
{}

void RequestHandler::ProcessBaseRequests(const json::Reader& reader) {
    std::vector<StopData> m_stops;
    std::vector<BusData> m_buses;

    const auto& requests {reader.GetBaseRequests().AsArray()};
    for (const json::Node& req : requests) {
        if (req.AsMap().at("type"s).AsString() == "Bus"sv) {
            m_buses.emplace_back(json::BusDataFromJSON(req));
        } else if (req.AsMap().at("type"s).AsString() == "Stop"sv) {
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
        m_transport_catalogue.AddBus(bd.name, bd.stops, bd.is_roundtrip);
    }
}

void RequestHandler::ProcessStatRequests(const json::Reader &reader, std::ostream& out){
    json::Array results;
    const auto& node {reader.GetStatRequests()};
    if (!node.IsArray()) return;
    const auto& requests {node.AsArray()};

    for (const json::Node& req : requests) {
        const std::string_view type {req.AsMap().at("type"s).AsString()};
        int id {req.AsMap().at("id"s).AsInt()};

        if (type == "Bus"sv) {
            const std::string& name {req.AsMap().at("name"s).AsString()};
            results.emplace_back(json::ToJSON(id, m_transport_catalogue.GetInfo(TransportCatalogue::BusQuery{name})));
        } else if (type == "Stop"sv) {
            const std::string& name {req.AsMap().at("name"s).AsString()};
            results.emplace_back(json::ToJSON(id, m_transport_catalogue.GetInfo(TransportCatalogue::StopQuery{name})));
        } else if (type == "Map"sv) {
            MapRenderer renderer(json::GetSettingsFromJSON(reader.GetRenderSettings()));
            std::stringstream out;
            renderer.Draw(m_transport_catalogue.GetBuses(), out);
            results.emplace_back(json::Dict{
                                     {"request_id"s, json::Node(id)},
                                     {"map"s, json::Node(out.str())},
                                 });
        }
    }

    if (!results.empty()) {
        json::PrintNode(json::Node{std::move(results)}, out);
        out << std::endl;
    }
}
