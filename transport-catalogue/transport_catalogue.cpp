#include "transport_catalogue.h"

#include <transport_catalogue.pb.h>

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

Info TransportCatalogue::GetBusInfo(int id, std::string_view name) const
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

Info TransportCatalogue::GetStopInfo(int id, std::string_view name) const
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

const std::deque<Stop>& TransportCatalogue::GetStops() const
{
    return m_dqstops;
}

bool TransportCatalogue::Serialize(std::ostream& output) const
{
    proto::TransportCatalogue proto_catalogue;
    std::unordered_map<std::string_view, size_t> stops_to_id;
    std::unordered_map<std::string_view, size_t> buses_to_id;

    size_t stop_id {0};
    for (const auto& stop : m_dqstops) {
        auto proto_stop = proto_catalogue.add_stops();
        stops_to_id.emplace(stop.name, stop_id);
        proto_stop->set_id(stop_id++);
        proto_stop->set_name(stop.name);
        proto_stop->set_lat(stop.coord.lat);
        proto_stop->set_lng(stop.coord.lng);
    }

    size_t bus_id {0};
    for (const auto& bus : m_dqbuses) {
        auto proto_bus = proto_catalogue.add_buses();
        buses_to_id.emplace(bus.name, bus_id);
        proto_bus->set_id(bus_id++);
        proto_bus->set_name(bus.name);
        for (const auto& stop : bus.stops) {
            proto_bus->add_stops(stops_to_id.at(stop->name));
        }
        proto_bus->set_is_roundtrip(bus.is_roundtrip);
        proto_bus->set_num_unique(bus.num_unique);
        proto_bus->set_geo_length(bus.geo_length);
        proto_bus->set_route_length(bus.route_length);
    }

    for (const auto& [pair_stop, distance] : m_stops_distance) {
        auto proto_distance = proto_catalogue.add_distances();
        proto_distance->set_stop_first(stops_to_id.at(pair_stop.first));
        proto_distance->set_stop_second(stops_to_id.at(pair_stop.second));
        proto_distance->set_value(distance);
    }

    for (const auto& [stop, set_buses] : m_stop_to_buses) {
        auto proto_stop_to_buses = proto_catalogue.add_stop_to_buses();
        proto_stop_to_buses->set_stop_id(stops_to_id.at(stop));
        for(const auto& bus_name : set_buses) {
            proto_stop_to_buses->add_buses_id(buses_to_id.at(bus_name));
        }
    }

    return proto_catalogue.SerializeToOstream(&output);
}

bool TransportCatalogue::Deserialize(std::istream& input)
{
    proto::TransportCatalogue proto_catalogue;
    if (!proto_catalogue.ParseFromIstream(&input)) return false;

    const auto& proto_stops {proto_catalogue.stops()};
    const auto& proto_buses {proto_catalogue.buses()};
    std::unordered_map<size_t, StopPtrConst> id_to_stop;
    std::unordered_map<size_t, BusPtrConst> id_to_bus;

    for (const auto& proto_stop : proto_stops) {
        StopPtrConst stop_ptr {
            EmplaceStop({proto_stop.name(),
                        {proto_stop.lat(), proto_stop.lng()}
                        })
        };
        id_to_stop.emplace(proto_stop.id(), stop_ptr);
    }

    for (const auto& proto_distance : proto_catalogue.distances()) {
        SetDistance(id_to_stop.at(proto_distance.stop_first())->name,
                    id_to_stop.at(proto_distance.stop_second())->name,
                    proto_distance.value()
                    );
    }

    for (const auto& proto_bus : proto_buses) {
        std::vector<StopPtrConst> stops_ptrs;
        stops_ptrs.reserve(proto_bus.stops_size());

        for (const auto& stop_id : proto_bus.stops()) {
            stops_ptrs.emplace_back(id_to_stop.at(stop_id));
        }

        BusPtrConst bus_ptr {
            EmplaceBus({std::string(proto_bus.name()),
                        std::move(stops_ptrs),
                        proto_bus.num_unique(),
                        proto_bus.route_length(),
                        proto_bus.is_roundtrip()
                       })
        };
        id_to_bus.emplace(proto_bus.id(), bus_ptr);
    }

    for (const auto& proto_stop_to_buses : proto_catalogue.stop_to_buses()) {
        std::set<std::string_view> buses_set;
        for (const auto& bus_id : proto_stop_to_buses.buses_id()) {
            buses_set.insert(id_to_bus.at(bus_id)->name);
        }
        m_stop_to_buses.emplace(
                    id_to_stop.at(proto_stop_to_buses.stop_id())->name,
                    std::move(buses_set)
                    );
    }

    return true;
}


Info BusQuery::Get(const TransportCatalogue& catalogue) const
{
    return catalogue.GetBusInfo(request_id, name);
}

Info StopQuery::Get(const TransportCatalogue& catalogue) const
{
    return catalogue.GetStopInfo(request_id, name);
}

