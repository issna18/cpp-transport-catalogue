#include "transport_catalogue.h"

#include <unordered_set>

void TransportCatalogue::AddBus(const std::string_view bus_name,
                                const std::vector<std::string_view>& bus_stops) {
    std::vector<StopPtrConst> v_s;
    v_s.reserve(bus_stops.size());
    std::unordered_set<std::string_view> unique_stops;
    for (std::string_view name_stop : bus_stops) {
        StopPtrConst stop = m_names_stops.at(name_stop);
        v_s.emplace_back(stop);
        unique_stops.emplace(stop->name);
    }

    geo::Coordinates prev_coord = v_s.at(0)->coord;
    double distance {};
    for (auto it = v_s.begin() + 1; it != v_s.end(); it++) {
        geo::Coordinates current_coord {(*it)->coord};
        distance += ComputeDistance(prev_coord, current_coord);
        prev_coord = current_coord;
    }

    const Stop* prev_stop = v_s.at(0);
    int length {0};
    for (auto it = v_s.begin() + 1; it != v_s.end(); it++) {
        const Stop* current_stop {*it};
        PairStops key {prev_stop, current_stop};
        if (m_stops_distance.count(key) > 0) {
            length += m_stops_distance.at(key);
        } else {
            length += m_stops_distance.at({current_stop, prev_stop});
        }
        prev_stop = current_stop;
    }

    BusPtrConst bus = &m_dqbuses.emplace_back(Bus(std::string(bus_name), v_s,
                                                  unique_stops.size(), distance,
                                                  length));
    m_names_buses.emplace(bus->name, bus);
    for (const auto & stop_name : unique_stops) {
        m_stop_to_buses[stop_name].insert(bus->name);
    }
}

void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates& c) {
    StopPtrConst stop = &m_dqstops.emplace_back(Stop(std::string(name), c));
    m_names_stops.emplace(stop->name, stop);
}

void TransportCatalogue::SetDistance(std::string_view name,
                                     std::string_view other,
                                     int distance) {
    StopPtrConst stop = m_names_stops.at(name);
    StopPtrConst stop_other = m_names_stops.at(other);
    PairStops key {stop, stop_other};
    m_stops_distance[key] = distance;
}

TransportCatalogue::BusInfo TransportCatalogue::GetInfo(const BusQuery& query) const
{
    if (m_names_buses.count(query.name) == 0) {
        return BusInfo {ResultStatus::NotFound,
                        query.id,
                        query.name,
                        0,
                        0,
                        0,
                        0
        };
    }

    const Bus* bus_ptr {m_names_buses.at(query.name)};
    return BusInfo {ResultStatus::Success,
                    query.id,
                    bus_ptr->name,
                    bus_ptr->stops.size(),
                    bus_ptr->num_unique,
                    bus_ptr->geo_length,
                    bus_ptr->route_length
    };
}


TransportCatalogue::StopInfo TransportCatalogue::GetInfo(const StopQuery& query) const
{
    if (m_names_stops.count(query.name) == 0) {
        return StopInfo {ResultStatus::NotFound, query.id, query.name, {}};
    }

    if (m_stop_to_buses.count(query.name) == 0) {
        return StopInfo {ResultStatus::Success, query.id, query.name, {}};
    }

    return StopInfo {ResultStatus::Success,
                     query.id,
                     query.name,
                     {m_stop_to_buses.find(query.name)->second}
    };
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const
{
    return m_dqbuses;
}
