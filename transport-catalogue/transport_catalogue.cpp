#include "transport_catalogue.h"

#include <unordered_set>

void TransportCatalogue::AddBus(const std::string_view bus_name,
                                const std::vector<std::string_view>& bus_stops,
                                bool is_roudtrip) {
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
                                                  length, is_roudtrip));
    m_names_buses.emplace(bus->name, bus);
    for (const auto & stop_name : unique_stops) {
        m_stop_to_buses[stop_name].insert(bus->name);
    }
}

void TransportCatalogue::AddBuses(const std::vector<BusData>& buses) {
    for (const BusData& bd : buses) {
        AddBus(bd.name, bd.stops, bd.is_roundtrip);
    }
}

void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates& c) {
    StopPtrConst stop = &m_dqstops.emplace_back(Stop(std::string(name), c));
    m_names_stops.emplace(stop->name, stop);
}

void TransportCatalogue::AddStops(const std::vector<StopData>& stops) {
    for (const StopData& sd: stops) {
        AddStop(sd.name, sd.coordinates);
    }
    for (const StopData& sd : stops) {
        for (const auto& [other, distance] : sd.adjacent) {
            SetDistance(sd.name, other, distance);
        }
    }
}

void TransportCatalogue::SetDistance(std::string_view name,
                                     std::string_view other,
                                     int distance) {
    StopPtrConst stop = m_names_stops.at(name);
    StopPtrConst stop_other = m_names_stops.at(other);
    PairStops key {stop, stop_other};
    m_stops_distance[key] = distance;
}

BusInfo TransportCatalogue::GetBusInfo(int id, std::string_view name) const
{
    if (m_names_buses.count(name) == 0) {
        return BusInfo {
            id,
            ResultStatus::NotFound,
            name,
            0,
            0,
            0,
            0
        };
    }

    const Bus* bus_ptr {m_names_buses.at(name)};
    return BusInfo {
        id,
        ResultStatus::Success,
        bus_ptr->name,
        bus_ptr->stops.size(),
        bus_ptr->num_unique,
        bus_ptr->geo_length,
        bus_ptr->route_length
    };
}

StopInfo TransportCatalogue::GetStopInfo(int id, std::string_view name) const
{
    if (m_names_stops.count(name) == 0) {
        return StopInfo {id, ResultStatus::NotFound, name, {}};
    }

    if (m_stop_to_buses.count(name) == 0) {
        return StopInfo {id, ResultStatus::Success, name, {}};
    }

    return StopInfo {id, ResultStatus::Success,
                     name,
                     {m_stop_to_buses.find(name)->second}
    };
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const
{
    return m_dqbuses;
}

Info BusQuery::Get(const TransportCatalogue& catalogue) const {
    return catalogue.GetBusInfo(request_id, name);
}

Info StopQuery::Get(const TransportCatalogue& catalogue) const {
    return catalogue.GetStopInfo(request_id, name);
}

Info MapQuery::Get(const TransportCatalogue& catalogue) const {
    return MapInfo {request_id, catalogue.GetBuses()};
}

