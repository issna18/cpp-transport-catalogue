#pragma once

#include "json.h"
#include "geo.h"
#include "svg.h"

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct StopData
{
    StopData(const json::Node& node);
    std::string_view name;
    geo::Coordinates coordinates;
    std::unordered_map<std::string_view, int> adjacent;
};

struct BusData
{
    BusData(const json::Node& node);
    std::string_view name;
    std::vector<std::string_view> stops;
    bool is_roundtrip {false};
};

class Stop
{
public:
    Stop(std::string&& n, geo::Coordinates&& c);
    Stop(const std::string& n, const geo::Coordinates& c);

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
    bool is_roundtrip {false};
    size_t num_unique;
    double geo_length {0.0};
    int route_length;

    bool operator==(const Bus& other) const;
};

using BusPtrConst = const Bus*;

struct RenderSettings
{
    RenderSettings() = default;
    RenderSettings(const json::Node& node);
    bool FromJSON(const json::Node& node);
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
    RoutingSettings(const json::Node& node);
    int bus_wait_time {1};
    double bus_velocity {1.0};
};

struct SerializationSettings
{
    std::string file_name;

    SerializationSettings(const json::Node& node)
        : file_name {node.AsDict().at("file").AsString()}
    {}
};

struct Info {
    virtual ~Info() {};
    virtual json::Node ToJSON(int request_id) const = 0;
};

struct ErrorInfo : public Info {
    json::Node ToJSON(int request_id) const override;
};

struct BusInfo : public Info {
    BusInfo(const std::string_view a_name,
            size_t a_num_stops,
            size_t a_num_unique,
            double a_geo_length,
            int a_route_length)
        : name {a_name},
          num_stops {a_num_stops},
          num_unique {a_num_unique},
          geo_length {a_geo_length},
          route_length {a_route_length}
    {}
    const std::string_view name;
    size_t num_stops {0};
    size_t num_unique {0};
    double geo_length {0.0};
    int route_length {0};
    json::Node ToJSON(int request_id) const override;
};

struct StopInfo : public Info {
    StopInfo(std::string_view a_name, const std::set<std::string_view>& a_buses)
        : name {a_name},
          buses {a_buses}
    {}
    StopInfo(std::string_view a_name)
        : name {a_name}
    {}
    const std::string_view name;
    const std::set<std::string_view> buses;
    json::Node ToJSON(int request_id) const override;
};

struct MapInfo : public Info {
    MapInfo(std::string a_map)
        : map {std::move(a_map)}
    {}

    std::string map;
    json::Node ToJSON(int request_id) const override;
};


struct RouteInfo : public Info {
    struct RouteItem {
        const std::string_view name;
        bool is_wait {false};
        double time {0.0};
        size_t span_count {0};
        json::Node ToJSON() const;
    };

    RouteInfo(double a_total_time, std::vector<RouteItem>&& a_items)
        : total_time {a_total_time},
          items {std::move(a_items)}
    {}

    double total_time;
    std::vector<RouteItem> items;
    json::Node ToJSON(int request_id) const override;
};
