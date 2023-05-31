#include "domain.h"

Stop::Stop(std::string&& n, const geo::Coordinates& c)
    : name {std::move(n)},
      coord {c}
{}

bool Stop::operator==(const Stop& other) const {
    return name == other.name;
}

Bus::Bus(std::string&& n,
         std::vector<StopPtrConst>&& s,
         size_t num_u,
         int length,
         bool is_round)
    : name {std::move(n)},
      stops {std::move(s)},
      num_unique {num_u},
      route_length {length},
      is_roundtrip {is_round}
{
    geo::Coordinates prev_coord = stops.at(0)->coord;
    for (auto it = stops.cbegin() + 1; it != stops.cend(); it++) {
        geo::Coordinates current_coord {(*it)->coord};
        geo_length += ComputeDistance(prev_coord, current_coord);
        prev_coord = current_coord;
    }
}

bool Bus::operator==(const Bus& other) const {
    return name == other.name;
}

size_t PairStopsHasher::operator() (const PairStops stops) const {
    size_t h_first = hasher(stops.first);
    size_t h_second = hasher(stops.second);
    return h_first + h_second * 37;
}
