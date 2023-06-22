#include "json_reader.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace json {

Reader::Reader(std::istream& in)
    : m_json {Load(in)}
{}

const Node& Reader::GetMainRequest(const std::string& key) const {
    if (m_json.GetRoot().AsDict().count(key)) {
        return m_json.GetRoot().AsDict().at(key);
    }
    return empty;
}

std::pair<std::vector<StopData>, std::vector<BusData>> Reader::GetStopsAndBuses() const {
    const auto& requests {GetMainRequest("base_requests"s).AsArray()};
    std::vector<StopData> stops;
    std::vector<BusData> buses;

    for (const json::Node& req : requests) {
        if (req.AsDict().at("type"s).AsString() == "Bus"sv) {
            buses.emplace_back(BusData(req));
        } else if (req.AsDict().at("type"s).AsString() == "Stop"sv) {
            stops.emplace_back(StopData(req));
        } else {
            throw std::invalid_argument("Invalid Request Type");
        }
    }

    return std::make_pair(std::move(stops), std::move(buses));
}

std::vector<Query> Reader::GetQueries() const {
    const auto& requests {GetMainRequest("stat_requests"s).AsArray()};
    std::vector<Query> queries;

    for (const json::Node& req : requests) {
        const std::string_view type {req.AsDict().at("type"s).AsString()};
        int id {req.AsDict().at("id"s).AsInt()};

        if (type == "Bus"sv) {
            queries.emplace_back(BusQuery {id, req.AsDict().at("name"s).AsString()});
        } else if (type == "Stop"sv) {
            queries.emplace_back(StopQuery {id, req.AsDict().at("name"s).AsString()});
        } else if (type == "Map"sv) {
            queries.emplace_back(MapQuery {id});
        } else if (type == "Route"sv) {
            queries.emplace_back(RouteQuery {id,
                                 req.AsDict().at("from"s).AsString(),
                                 req.AsDict().at("to"s).AsString()
                                 });
        }
    }
    return queries;
}

RenderSettings Reader::GetRenderSettings() const {
    return RenderSettings(GetMainRequest("render_settings"s));
}

SerializationSettings Reader::GetSerializationSettings() const {
    return SerializationSettings(GetMainRequest("serialization_settings"s));
}

RoutingSettings MakeRoutingSettingsFromJSON(const Node& node) {
    if (!node.IsDict()) return {};
    const auto& json {node.AsDict()};
    return {
          json.at("bus_wait_time"s).AsInt(),
          json.at("bus_velocity"s).AsDouble()
    };
}

}
