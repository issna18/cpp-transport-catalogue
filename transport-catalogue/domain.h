#pragma once

#include "geo.h"
#include "svg.h"
#include "json_builder.h"

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct StopData
{
    std::string_view name;
    geo::Coordinates coordinates;
    std::unordered_map<std::string_view, int> adjacent;
};

struct BusData
{
    std::string_view name;
    std::vector<std::string_view> stops;
    bool is_roundtrip {false};
};

class Stop
{
public:
    Stop(std::string&& n, const geo::Coordinates& c);

    std::string name;
    geo::Coordinates coord;

    bool operator==(const Stop& other) const;
};

using StopPtrConst = const Stop*;
using PairStops = std::pair<std::string_view, std::string_view>;

struct PairStopsHasher {
    size_t operator() (const PairStops stops) const;

private:
    std::hash<std::string_view> hasher;
};

class Bus {
public:
    Bus(std::string&& n, std::vector<StopPtrConst>&& s,
        size_t num_u, int r_len, bool is_round);

    std::string name;
    std::vector<StopPtrConst> stops;
    size_t num_unique;
    double geo_length {0.0};
    int route_length;
    bool is_roundtrip {false};

    bool operator==(const Bus& other) const;
};

using BusPtrConst = const Bus*;

struct RenderSettings
{
    double width {0.0};
    double height {0.0};
    double padding {0.0};
    double line_width {0.0};
    double stop_radius {0.0};

    int bus_label_font_size {0};
    svg::Point bus_label_offset {0.0, 0.0};

    int stop_label_font_size {0};
    svg::Point stop_label_offset {0.0, 0.0};

    svg::Color underlayer_color {};
    double underlayer_width {0.0};

    std::vector<svg::Color> color_palette {};
};

struct RoutingSettings
{
    int bus_wait_time {1};
    double bus_velocity {1.0};
};

enum class ResultStatus
{
    Success,
    NotFound
};

struct BusInfo {
    int request_id;
    ResultStatus status {ResultStatus::NotFound};
    const std::string_view name;
    size_t num_stops {0};
    size_t num_unique {0};
    double geo_length {0.0};
    int route_length {0};
};

struct StopInfo {
    int request_id;
    ResultStatus status {ResultStatus::NotFound};
    const std::string_view name;
    const std::set<std::string_view> buses;
};

struct MapInfo {
    int request_id;
    const std::deque<Bus> buses;
};

struct RouteItem {
    const std::string_view name;
    bool is_wait {false};
    double time {0.0};
    size_t span_count {0};
    json::Node to_json() const {
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
};

struct RouteInfo {
    int request_id;
    ResultStatus status {ResultStatus::NotFound};
    double total_time {0.0};
    std::vector<RouteItem> items {};
    json::Node to_json() const {
        using namespace std::string_literals;
        return json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(request_id)
                .Key("total_time"s).Value(total_time)
                .Key("items"s).Value([this]()
                                {
                                    json::Array value;
                                    for (const auto& item : items) {
                                        value.emplace_back(item.to_json());
                                    }
                                    return value;
                                }())
                .EndDict()
                .Build();
    }
};

using Info = std::variant<BusInfo, StopInfo, MapInfo, RouteInfo>;
