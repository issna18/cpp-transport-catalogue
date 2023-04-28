#pragma once

#include "domain.h"

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

class TransportCatalogue
{
public:
    enum class ResultStatus;

    using MapInfo = const std::deque<Bus>;
    struct BusInfo;
    struct StopInfo;
    struct BusQuery;
    struct StopQuery;

    void AddBus(const std::string_view bus_name,
                const std::vector<std::string_view>& bus_stops,
                bool is_roundtrip = false);
    void AddStop(std::string_view name, const geo::Coordinates& c);
    void SetDistance(std::string_view name,
                     std::string_view other, int distance);

    BusInfo GetInfo(const BusQuery& query) const;
    StopInfo GetInfo(const StopQuery& query) const;

    MapInfo& GetBuses() const;

    enum class ResultStatus
    {
        Success,
        NotFound
    };

    struct BusInfo {
        ResultStatus status {ResultStatus::NotFound};
        const std::string_view name;
        size_t num_stops {0};
        size_t num_unique {0};
        double geo_length {0.0};
        int route_length {0};
    };

    struct StopInfo {
        ResultStatus status {ResultStatus::NotFound};
        const std::string_view name;
        const std::set<std::string_view> buses;
    };

    struct BusQuery { std::string name; };
    struct StopQuery { std::string name; };

private:
    std::deque<Stop> m_dqstops;
    std::unordered_map<std::string_view, StopPtrConst> m_names_stops;
    std::deque<Bus> m_dqbuses;
    std::unordered_map<std::string_view, BusPtrConst> m_names_buses;
    std::unordered_map<std::string_view, std::set<std::string_view>> m_stop_to_buses;
    std::unordered_map<PairStops, int, PairStopsHasher> m_stops_distance;
};

