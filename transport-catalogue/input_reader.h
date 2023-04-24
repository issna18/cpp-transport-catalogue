#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

template <typename F, typename S>
std::ostream& operator<<(std::ostream& out, const std::pair<F, S> p) {
    out << '(' << p.first << ", " << p.second << ')';
    return out;
}

template <typename Container>
std::ostream& Print(std::ostream& out, const Container& container)
{
     bool is_first = true;
    for (const auto& element : container) {
        if (!is_first) {
            out << " ";
        }
        is_first = false;
        out << element;
    }
    return out;
}

template <typename ElementT>
std::ostream& operator<<(std::ostream& out, const std::set<ElementT> container) {
    return Print(out, container);
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& out, const std::unordered_map<K, V> container) {
    out << '<';
    return Print(out, container) << '>';
}

namespace Reader {

class Input
{
public:
    Input(TransportCatalogue& transport_cataloge);
    void Read(const json::Document& jdoc);

private:
    void ProcessBaseRequests(const json::Node& requests);
    StopData ParseStop(const json::Node& node) const;
    BusData ParseBus(const json::Node& node) const;
    void FillCatalogue() const;

    std::vector<StopData> m_stops;
    std::vector<BusData> m_buses;
    TransportCatalogue& m_transport_cataloge;
};
}
