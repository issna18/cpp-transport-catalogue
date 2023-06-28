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

    std::string_view prev_stop_name {v_s.front()->name};
    int length {0};
    for (auto it = v_s.cbegin() + 1; it != v_s.cend(); it++) {
        const std::string_view current_stop_name {(*it)->name};
        length += GetDistance(prev_stop_name, current_stop_name);
        prev_stop_name = current_stop_name;
    }

    BusPtrConst bus {EmplaceBus({std::string(bus_name),
                                 std::move(v_s),
                                 unique_stops.size(),
                                 length,
                                 is_roudtrip})};
    for (const auto & stop_name : unique_stops) {
        m_stop_to_buses[stop_name].insert(bus->name);
    }
}

BusPtrConst TransportCatalogue::EmplaceBus(Bus&& bus) {
    BusPtrConst bus_ptr = &m_dqbuses.emplace_back(std::move(bus));
    m_names_buses.emplace(bus_ptr->name, bus_ptr);
    return bus_ptr;
}

void TransportCatalogue::AddBuses(const std::vector<BusData>& buses) {
    for (const BusData& bd : buses) {
        AddBus(bd.name, bd.stops, bd.is_roundtrip);
    }
}

void TransportCatalogue::AddStop(std::string_view name, const geo::Coordinates& c) {
    EmplaceStop({std::string(name), c});
}

StopPtrConst TransportCatalogue::EmplaceStop(Stop&& stop) {
    StopPtrConst stop_ptr = &m_dqstops.emplace_back(std::move(stop));
    m_names_stops.emplace(stop_ptr->name, stop_ptr);
    return stop_ptr;
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
    m_stops_distance[{name, other}] = distance;
}

int TransportCatalogue::GetDistance(std::string_view name,
                                    std::string_view other) const {
    PairStops key {name, other};
    if (m_stops_distance.count(key) > 0) {
        // Точно известно расстояние от предыдущей до текущей остановки
        return m_stops_distance.at(key);
    }
    // Расстояние не задано, считаем равным от текущей до предыдущей
    return m_stops_distance.at({other, name});

}

std::unique_ptr<Info> TransportCatalogue::GetBusInfo(std::string_view name) const
{
    if (m_names_buses.count(name) == 0) {
        return std::make_unique<ErrorInfo>();
    }

    const Bus* bus_ptr {m_names_buses.at(name)};
    return std::make_unique<BusInfo>(
        bus_ptr->name,
        bus_ptr->stops.size(),
        bus_ptr->num_unique,
        bus_ptr->geo_length,
        bus_ptr->route_length);
}

std::unique_ptr<Info> TransportCatalogue::GetStopInfo(std::string_view name) const
{
    if (m_names_stops.count(name) == 0) {
        return std::make_unique<ErrorInfo>();
    }

    if (m_stop_to_buses.count(name) == 0) {
        return std::make_unique<StopInfo>(name);
    }

    return std::make_unique<StopInfo>(name, m_stop_to_buses.find(name)->second);
}

BusPtrConst TransportCatalogue::GetBus(std::string_view name) const
{
    if (m_names_buses.count(name) == 0) return nullptr;
    return m_names_buses.at(name);
}

StopPtrConst TransportCatalogue::GetStop(std::string_view name) const
{
    if (m_names_stops.count(name) == 0) return nullptr;
    return m_names_stops.at(name);
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const
{
    return m_dqbuses;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const
{
    return m_dqstops;
}

std::unique_ptr<Info> BusQuery::Request(const TransportCatalogue& catalogue) const
{
    return catalogue.GetBusInfo(name);
}

std::unique_ptr<Info> StopQuery::Request(const TransportCatalogue& catalogue) const
{
    return catalogue.GetStopInfo(name);
}

