#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

struct StopData
{
    std::string name;
    Coordinates coordinates;
    std::unordered_map<std::string, int> adjacent;
};

using BusData = std::pair<std::string, std::vector<std::string>>;

class TransportCatalogue
{
public:

    void AddBus(const std::string_view bus_name,
                const std::vector<std::string>& bus_stops);
    void AddStop(std::string_view name, const Coordinates& c);
    void AddAdjacent(std::string_view name,
                     std::string_view other, int distance);

    enum class ResultStatus
    {
        Success,
        NotFound
    };

    struct BusInfo {
        ResultStatus status {ResultStatus::NotFound};
        size_t num_stops {0};
        size_t num_unique {0};
        double geo_length {0.0};
        int route_length {0};
    };

    struct StopInfo {
        ResultStatus status {ResultStatus::NotFound};
        const std::set<std::string_view> buses;
    };

    BusInfo GetBusInfo(std::string_view bus_name);
    StopInfo GetStopInfo(std::string_view stop_name);

private:

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

struct PairStopsHasher {
    size_t operator() (const PairStops stops) const;

private:
    std::hash<StopPtrConst> hasher;
};

private:
    std::deque<Stop> m_dqstops;
    std::unordered_map<std::string_view, StopPtrConst> m_names_stops;
    std::deque<Bus> m_dqbuses;
    std::unordered_map<std::string_view, BusPtrConst> m_names_buses;
    std::unordered_map<std::string_view, std::set<std::string_view>> m_stop_to_buses;
    std::unordered_map<PairStops, int, PairStopsHasher> m_stops_distance;
};

