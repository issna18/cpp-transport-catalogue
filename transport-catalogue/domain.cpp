#include "domain.h"
#include "json_builder.h"

using namespace std::string_literals;

StopData::StopData(const json::Node& node)
    : name {node.AsDict().at("name").AsString()}
{
    auto c_lat {node.AsDict().at("latitude"s).AsDouble()};
    auto c_long {node.AsDict().at("longitude"s).AsDouble()};

    const json::Node& distances {node.AsDict().at("road_distances"s)};
    for (const auto& entry : distances.AsDict()) {
        std::pair<std::string_view, int> adj {entry.first, entry.second.AsInt()};
        adjacent.emplace(std::move(adj));
    }

    coordinates = geo::Coordinates{c_lat, c_long};
}

BusData::BusData(const json::Node& node)
    : name {node.AsDict().at("name"s).AsString()},
      is_roundtrip {node.AsDict().at("is_roundtrip"s).AsBool()}
{
    for (const auto& stop_node : node.AsDict().at("stops"s).AsArray()) {
        stops.emplace_back(stop_node.AsString());
    }

    if (!is_roundtrip) {
        std::vector<std::string_view> all_stops;
        all_stops.reserve(stops.size() * 2 - 1);
        all_stops.insert(all_stops.begin(), stops.begin(), stops.end());
        std::move(stops.rbegin() + 1, stops.rend(), std::back_inserter(all_stops));
        stops = std::move(all_stops);
    }
}

Stop::Stop(std::string&& n, geo::Coordinates&& c)
    : name {std::move(n)},
      coord {std::move(c)}
{}

Stop::Stop(const std::string& n,const geo::Coordinates& c)
    : name {n},
      coord {c}
{}


bool Stop::operator==(const Stop& other) const {
    return name == other.name;
}

Bus::Bus(std::string&& n,
         std::vector<StopPtrConst>&& s,
         size_t num_u,
         int length,
         bool is_round)
    : name {std::move(n)},
      stops {std::move(s)},
      is_roundtrip {is_round},
      num_unique {num_u},
      route_length {length}
{
    geo::Coordinates prev_coord = stops.at(0)->coord;
    for (auto it = stops.cbegin() + 1; it != stops.cend(); it++) {
        geo::Coordinates current_coord {(*it)->coord};
        geo_length += ComputeDistance(prev_coord, current_coord);
        prev_coord = current_coord;
    }
}

bool Bus::operator==(const Bus& other) const {
    return name == other.name;
}

size_t PairStopsHasher::operator() (const PairStops stops) const {
    size_t h_first = hasher(stops.first);
    size_t h_second = hasher(stops.second);
    return h_first + h_second * 37;
}

json::Node BusInfo::ToJSON() const {
    using namespace std::string_literals;
    return json::Builder{}
    .StartDict()
    .Key("request_id"s).Value(request_id)
    .Key("curvature"s).Value(route_length / geo_length)
    .Key("route_length"s).Value(route_length)
    .Key("stop_count"s).Value(static_cast<int>(num_stops))
    .Key("unique_stop_count"s).Value(static_cast<int>(num_unique))
    .EndDict()
    .Build();
}

json::Node StopInfo::ToJSON() const {
    using namespace std::string_literals;
    return json::Builder{}
    .StartDict()
    .Key("request_id"s).Value(request_id)
    .Key("buses"s).Value([this]()
    {
    json::Array value;
    for (const auto bus : buses) {
        value.emplace_back(std::string(bus));
    }
    return value;
}())
.EndDict()
.Build();
}

json::Node MapInfo::ToJSON() const {
    using namespace std::string_literals;
    return json::Builder{}
    .StartDict()
    .Key("request_id"s).Value(request_id)
    .Key("map"s).Value(map)
    .EndDict()
    .Build();
}


json::Node RouteItem::ToJSON() const {
    using namespace std::string_literals;
    auto item = json::Builder{}
            .StartDict()
            .Key(is_wait ? "stop_name"s : "bus"s).Value(std::string(name))
            .Key("time"s).Value(time)
            .Key("type"s).Value(is_wait ? "Wait"s : "Bus"s)
            .EndDict()
            .Build();

    if (!is_wait) item.AsDict().emplace("span_count", json::Node(static_cast<int>(span_count)));
    return item;
}

json::Node RouteInfo::ToJSON() const {
    using namespace std::string_literals;
    return json::Builder{}
    .StartDict()
    .Key("request_id"s).Value(request_id)
    .Key("total_time"s).Value(total_time)
    .Key("items"s).Value([this]()
    {
    json::Array value;
    for (const auto& item : items) {
        value.emplace_back(item.ToJSON());
    }
    return value;
}())
.EndDict()
.Build();
}

