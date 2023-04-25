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

const Node& Reader::GetRenderSettings() const {
    return m_json.GetRoot().AsMap().at("render_settings");
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
    return {node.AsMap().at("name"s).AsString(), geo::Coordinates{c_lat, c_long}, std::move(adjacent)};
}


RenderSettings GetSettingsFromJSON(const Node& node) {

    auto ColorFromJSON = [](const Node& n) {
        if (n.IsArray() && n.AsArray().size() == 4) {
            const auto& array {n.AsArray()};
            return svg::Color(svg::Rgba(static_cast<uint8_t>(array[0].AsInt()),
                                        static_cast<uint8_t>(array[1].AsInt()),
                                        static_cast<uint8_t>(array[2].AsInt()),
                                        array[3].AsDouble()
                                        )
            );
        } else if (n.IsArray() && n.AsArray().size() == 3) {
            const auto& array {n.AsArray()};
            return svg::Color(svg::Rgb(static_cast<uint8_t>(array[0].AsInt()),
                                       static_cast<uint8_t>(array[1].AsInt()),
                                       static_cast<uint8_t>(array[2].AsInt())
                                       )
            );
        } else if (n.IsString()){
            return svg::Color(n.AsString());
        }
        return svg::Color{};
    };

    const auto& json {node.AsMap()};
    RenderSettings settings;
    settings.width = json.at("width"s).AsDouble();
    settings.height = json.at("height"s).AsDouble();
    settings.padding = json.at("padding"s).AsDouble();
    settings.line_width = json.at("line_width"s).AsDouble();
    settings.stop_radius = json.at("stop_radius"s).AsDouble();

    settings.bus_label_font_size = json.at("bus_label_font_size"s).AsInt();
    const auto& bl_offset {json.at("bus_label_offset"s).AsArray()};
    settings.bus_label_offset = {bl_offset[0].AsDouble(), bl_offset[1].AsDouble()};

    settings.stop_label_font_size = json.at("stop_label_font_size"s).AsInt();
    const auto& sl_offset {json.at("stop_label_offset"s).AsArray()};
    settings.stop_label_offset = {sl_offset[0].AsDouble(), sl_offset[1].AsDouble()};

    settings.underlayer_color = ColorFromJSON(json.at("underlayer_color"s));
    settings.underlayer_width = json.at("underlayer_width").AsDouble();

    const auto& palette {json.at("color_palette"s).AsArray()};
    for (const Node& j_color : palette) {
        settings.color_palette.emplace_back(ColorFromJSON(j_color));
    }

    return settings;
}


}
