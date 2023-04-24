#include "json_reader.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace json {

using namespace std::string_literals;

Reader::Reader(std::istream& in)
    : m_json {Load(in)}
{}

const Node& Reader::GetBaseRequests() const {
    return m_json.GetRoot().AsMap().at("base_requests");
};

const Node& Reader::GetStatRequests() const {
    return m_json.GetRoot().AsMap().at("stat_requests");
};


Node ToJSON(const TransportCatalogue::BusInfo& info) {
    Dict result;
    result["request_id"s] = Node(info.request_id);
    if (info.status == TransportCatalogue::ResultStatus::NotFound) {
        result["error_message"s] = Node("not found"s);
        return Node(std::move(result));
    }
    result["curvature"s] = Node(info.route_length/info.geo_length);
    result["route_length"s] = Node(info.route_length);
    result["stop_count"s] = Node(static_cast<int>(info.num_stops));
    result["unique_stop_count"s] = Node(static_cast<int>(info.num_unique));

    return Node(std::move(result));
}

Node ToJSON(const TransportCatalogue::StopInfo& info) {
    Dict result;

    result["request_id"s] = Node(info.request_id);
    if (info.status == TransportCatalogue::ResultStatus::NotFound) {
        result["error_message"s] = Node("not found"s);
        return Node(std::move(result));
    }
    Array buses;
    for (const auto& bus : info.buses) {
        buses.push_back(Node(std::string(bus)));
    }
    result["buses"s] = Node(std::move(buses));

    return Node(result);
}

BusData BusDataFromJSON(const Node& node) {
    std::vector<std::string_view> stops;
    bool is_roundtrip {node.AsMap().at("is_roundtrip"s).AsBool()};

    for (const auto& stop_node : node.AsMap().at("stops"s).AsArray()) {
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

StopData StopDataFromJSON(const Node& node) {
    auto c_lat {node.AsMap().at("latitude"s).AsDouble()};
    auto c_long {node.AsMap().at("longitude"s).AsDouble()};

    std::unordered_map<std::string_view, int> adjacent;

    const Node& distances {node.AsMap().at("road_distances"s)};
    for (const auto& entry : distances.AsMap()) {
        std::pair<std::string_view, int> adj {entry.first, entry.second.AsInt()};
        adjacent.emplace(std::move(adj));
    }
    return {node.AsMap().at("name"s).AsString(), Coordinates{c_lat, c_long}, std::move(adjacent)};
}

}
