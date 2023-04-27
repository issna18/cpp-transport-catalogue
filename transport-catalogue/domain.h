#pragma once
/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
#include "geo.h"
#include "svg.h"

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

struct QueryCatalogue
{
    int id {0};
    std::string name;
};

struct BusQuery : public QueryCatalogue {};
struct StopQuery : public QueryCatalogue {};

class Stop
{
public:
    Stop(const std::string& n, const geo::Coordinates& c);

    std::string name;
    geo::Coordinates coord;

    bool operator==(const Stop& other) const;
};

using StopPtrConst = const Stop*;
using PairStops = std::pair<StopPtrConst, StopPtrConst>;

struct PairStopsHasher {
    size_t operator() (const PairStops stops) const;

private:
    std::hash<StopPtrConst> hasher;
};

class Bus {
public:
    Bus(const std::string& n, const std::vector<StopPtrConst>& s,
        size_t num_u, double g_len, int r_len, bool is_round);

    std::string name;
    std::vector<StopPtrConst> stops;
    size_t num_unique;
    double geo_length;
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
    std::string font_family {"Verdana"};
};
