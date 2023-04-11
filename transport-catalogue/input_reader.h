#pragma once

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

std::string_view LeftStrip(std::string_view sv);
std::string_view RightStrip(std::string_view sv);
std::string_view Strip(std::string_view sv);
std::pair<std::string, std::string_view> ParseString(std::string_view line, char delimiter);
std::pair<int, std::string_view> ParseInt(std::string_view line, char delimiter);
std::pair<double, std::string_view> ParseDouble(std::string_view line, char delimiter);

class Input
{
public:
    Input(std::istream& input);
    void FillCatalogue(TransportCatalogue& tc);

private:
    void ProcessLine(std::string_view line);
    static std::pair<std::pair<std::string, int>, std::string_view> ParseAdjacent(std::string_view line);
    StopData ParseStop(std::string_view line) const;
    BusData ParseBus(std::string_view line) const;

    std::vector<StopData> m_stops;
    std::vector<BusData> m_buses;
    std::string_view m_stop_delimiters {">-"};
};
}
