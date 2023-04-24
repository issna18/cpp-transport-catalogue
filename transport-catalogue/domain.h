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

#include <string>
#include <unordered_map>
#include <vector>

struct StopData
{
    std::string_view name;
    Coordinates coordinates;
    std::unordered_map<std::string_view, int> adjacent;
};

using BusData = std::pair<std::string_view, std::vector<std::string_view>>;


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
    Stop(const std::string& n, const Coordinates& c);

    std::string name;
    Coordinates coord;

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
        size_t num_u, double g_len, int r_len);

    std::string name;
    std::vector<StopPtrConst> stops;
    size_t num_unique;
    double geo_length;
    int route_length;

    bool operator==(const Bus& other) const;
};

using BusPtrConst = const Bus*;
