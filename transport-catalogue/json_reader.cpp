#include "json_reader.h"

#include <iostream>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace json {

using namespace std::string_literals;
using namespace std::string_view_literals;

Reader::Reader(TransportCatalogue& transport_cataloge)
    : m_transport_cataloge {transport_cataloge}
{}

void Reader::Read(const json::Document& jdoc) {
    ProcessRequests(jdoc.GetRoot().AsMap().at("base_requests"));
    FillCatalogue();
}

void Reader::FillCatalogue() const {
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

void Reader::ProcessRequests(const json::Node& requests) {
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

StopData Reader::ParseStop(const json::Node& node) const {
    auto c_lat {node.AsMap().at("latitude").AsDouble()};
    auto c_long {node.AsMap().at("longitude").AsDouble()};

    std::unordered_map<std::string_view, int> adjacent;

    const json::Node& distances {node.AsMap().at("road_distances")};
    for (const auto& entry : distances.AsMap()) {
        std::pair<std::string_view, int> adj {entry.first, entry.second.AsInt()};
        adjacent.emplace(std::move(adj));
    }
    return {node.AsMap().at("name"s).AsString(), Coordinates{c_lat, c_long}, std::move(adjacent)};
}

BusData Reader::ParseBus(const json::Node& node) const {
    std::vector<std::string_view> stops;
    bool is_roundtrip {node.AsMap().at("is_roundtrip"s).AsBool()};

    for (const auto& stop_node : node.AsMap().at("stops").AsArray()) {
        stops.emplace_back(stop_node.AsString());
    }

    if (!is_roundtrip) {
        std::vector<std::string_view> all_stops;
        all_stops.reserve(stops.size() * 2 - 1);
        all_stops.insert(all_stops.begin(), stops.begin(), stops.end());
        std::move(stops.rbegin() + 1, stops.rend(), std::back_inserter(all_stops));
        stops = std::move(all_stops);
    }

    return {node.AsMap().at("name"s).AsString(), std::move(stops)};
}

}
