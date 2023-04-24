#include "domain.h"

Stop::Stop(const std::string& n, const Coordinates& c)
    : name {n},
      coord {c}
{}

bool Stop::operator==(const Stop& other) const {
    return name == other.name;
}

Bus::Bus(const std::string& n,
         const std::vector<StopPtrConst>& s,
         size_t num_u,
         double g_len,
         int r_len)
    : name {n},
      stops {s},
      num_unique {num_u},
      geo_length {g_len},
      route_length {r_len}
{}

bool Bus::operator==(const Bus& other) const {
    return name == other.name;
}

size_t PairStopsHasher::operator() (const PairStops stops) const {
    size_t h_first = hasher(stops.first);
    size_t h_second = hasher(stops.second);
    return h_first + h_second * 37;
}



