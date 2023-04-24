#include "json_reader.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace Reader {

using namespace std::string_literals;
using namespace std::string_view_literals;

Input::Input(TransportCatalogue& transport_cataloge)
    : m_transport_cataloge {transport_cataloge}
{}

void Input::Read(const json::Document& jdoc) {
    ProcessBaseRequests(jdoc.GetRoot().AsMap().at("base_requests"));
    FillCatalogue();
}

void Input::FillCatalogue() const {
    for (const StopData& sd: m_stops){
        //std::cout << sd.name << sd.coordinates.lat << std::endl;
        m_transport_cataloge.AddStop(sd.name, sd.coordinates);
    }
    for (const StopData& sd : m_stops) {
        for (const auto& [other, distance] : sd.adjacent){
            //std::cout << sd.name << "=>" << other  << " " << distance<< std::endl;
            m_transport_cataloge.SetDistance(sd.name, other, distance);
        }
    }
    for (const BusData& bd : m_buses){
        m_transport_cataloge.AddBus(bd.first, bd.second);
    }
}

void Input::ProcessBaseRequests(const json::Node& requests) {
    for (const json::Node& req : requests.AsArray()) {
        if (req.AsMap().at("type").AsString() == "Bus"s) {
            m_buses.emplace_back(ParseBus(req));
        } else if (req.AsMap().at("type").AsString() == "Stop"s) {
            m_stops.emplace_back(ParseStop(req));
        } else {
            throw std::invalid_argument("Invalid Request Type");
        }
    }
}

StopData Input::ParseStop(const json::Node& node) const {
    std::string name {node.AsMap().at("name"s).AsString()};
    auto c_lat {node.AsMap().at("latitude").AsDouble()};
    auto c_long {node.AsMap().at("longitude").AsDouble()};

    std::unordered_map<std::string, int> adjacent;

    const json::Node& distances {node.AsMap().at("road_distances")};
    for (const auto& entry : distances.AsMap()) {
        std::pair<std::string, int> adj {entry.first, entry.second.AsInt()};
        adjacent.emplace(std::move(adj));
    }
    return {std::move(name), Coordinates{c_lat, c_long}, std::move(adjacent)};
}

BusData Input::ParseBus(const json::Node& node) const {
    std::vector<std::string> stops;
    std::string name {node.AsMap().at("name"s).AsString()};
    bool is_roundtrip {node.AsMap().at("is_roundtrip"s).AsBool()};

    for (const auto& stop_node : node.AsMap().at("stops").AsArray()) {
        stops.emplace_back(stop_node.AsString());
    }

    if (!is_roundtrip) {
        std::vector<std::string> all_stops;
        all_stops.reserve(stops.size() * 2 - 1);
        all_stops.insert(all_stops.begin(), stops.begin(), stops.end());
        std::move(stops.rbegin() + 1, stops.rend(), std::back_inserter(all_stops));
        stops = std::move(all_stops);
    }

    return {std::move(name), std::move(stops)};
}

}