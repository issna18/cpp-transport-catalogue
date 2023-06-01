#pragma once

#include "domain.h"

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <unordered_map>

class TransportCatalogue
{
public:
    void AddBus(const std::string_view bus_name,
                const std::vector<std::string_view>& bus_stops,
                bool is_roundtrip = false);
    void AddBuses(const std::vector<BusData>& buses);

    void AddStop(std::string_view name, const geo::Coordinates& c);
    void AddStops(const std::vector<StopData>& stops);

    void SetDistance(std::string_view name,
                     std::string_view other, int distance);

    int GetDistance(std::string_view name,
                    std::string_view other) const;

    Info GetBusInfo(int id, std::string_view name) const;
    Info GetStopInfo(int id, std::string_view name) const;

    const std::deque<Bus>& GetBuses() const;
    const std::deque<Stop>& GetStops() const;

private:
    std::deque<Stop> m_dqstops;
    std::unordered_map<std::string_view, StopPtrConst> m_names_stops;
    std::deque<Bus> m_dqbuses;
    std::unordered_map<std::string_view, BusPtrConst> m_names_buses;
    std::unordered_map<std::string_view, std::set<std::string_view>> m_stop_to_buses;
    std::unordered_map<PairStops, int, PairStopsHasher> m_stops_distance;
};

struct BusQuery {
    int request_id;
    std::string name;
    Info Get(const TransportCatalogue& catalogue) const;
};

struct StopQuery {
    int request_id;
    std::string name;
    Info Get(const TransportCatalogue& catalogue) const;
};
