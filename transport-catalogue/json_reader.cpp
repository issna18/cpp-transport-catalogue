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

const Node& Reader::GetBaseRequests() const {
    if (m_json.GetRoot().AsDict().count("base_requests"s)) {
        return m_json.GetRoot().AsDict().at("base_requests"s);
    }
    return empty;
}

const Node& Reader::GetStatRequests() const {
    if (m_json.GetRoot().AsDict().count("stat_requests")) {
        return m_json.GetRoot().AsDict().at("stat_requests");
    }
    return empty;
}

const Node& Reader::GetRenderSettings() const {
    return m_json.GetRoot().AsDict().at("render_settings"s);
}

const Node& Reader::GetRoutingSettings() const {
    return m_json.GetRoot().AsDict().at("routing_settings"s);
}

const std::pair<std::vector<StopData>, std::vector<BusData>> Reader::GetStopsAndBuses() const {
    const auto& requests {GetBaseRequests().AsArray()};
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

const std::vector<Query> Reader::GetQueries() const {
    const auto& requests {GetStatRequests().AsArray()};
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

RenderSettings MakeRenderSettingsFromJSON(const Node& node) {

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

    const auto& json {node.AsDict()};
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
    settings.underlayer_width = json.at("underlayer_width"s).AsDouble();

    const auto& palette {json.at("color_palette"s).AsArray()};
    for (const Node& j_color : palette) {
        settings.color_palette.emplace_back(ColorFromJSON(j_color));
    }

    return settings;
}

RoutingSettings MakeRoutingSettingsFromJSON(const Node& node) {
    const auto& json {node.AsDict()};
    return {
          json.at("bus_wait_time"s).AsInt(),
          json.at("bus_velocity"s).AsDouble()
    };
}

}
