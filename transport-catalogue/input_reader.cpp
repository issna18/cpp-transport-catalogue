#include "input_reader.h"

#include <istream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace Reader {

std::string_view LeftStrip(std::string_view sv) {
    sv.remove_prefix(std::min(sv.find_first_not_of(" \t"), sv.size()));
    return sv;
}

std::string_view RightStrip(std::string_view sv) {
    std::size_t found = sv.find_last_not_of(" \t");
    if (found != std::string::npos) {
        sv.remove_suffix(sv.size() - found - 1);
    } else {
        sv.remove_suffix(sv.size());
    }
    return sv;
}

std::string_view Strip(std::string_view sv) {
    return LeftStrip(RightStrip(sv));
}

std::pair<std::string, std::string_view> ParseString(std::string_view line, char delimiter){
    size_t pos {line.find(delimiter)};
    if (pos == std::string::npos) {
        pos = line.size();
    }
    std::string_view parsed_sv {line.substr(0, pos)};
    pos = std::min(pos + 1, line.size());
    std::string_view tail {line.substr(pos, line.size())};
    return {std::string(parsed_sv), tail};
}

std::pair<int, std::string_view> ParseInt(std::string_view line, char delimiter){
    auto token = ParseString(line, delimiter);
    return {std::stoi(token.first), token.second};
}

std::pair<double, std::string_view> ParseDouble(std::string_view line, char delimiter) {
    auto token = ParseString(line, delimiter);
    return {std::stod(token.first), token.second};
}

Input::Input(std::istream& input) {
    std::string str;
    int queries;
    input >> queries;
    getline(input,str);

    for (int i {0}; i < queries; ++i) {
        std::getline(input, str);
        str = Strip(str);
        if (str.empty()) continue;
        ProcessLine(str);
    }
}

void Input::FillCatalogue(TransportCatalogue& tc) {
    for (const StopData& sd: m_stops){
        tc.AddStop(sd.name, sd.coordinates);
    }
    for (const StopData& sd : m_stops) {
        for (const auto& [other, distance] : sd.adjacent){
            tc.AddAdjacent(sd.name, other, distance);
        }
    }
    for (BusData& bd : m_buses){
        tc.AddBus(bd.first, bd.second);
    }
}

void Input::ProcessLine(std::string_view line) {
    if (line.front() == 'S') {
        m_stops.emplace_back(ParseStop(line));
    }
    if (line.front() == 'B') {
        m_buses.emplace_back(ParseBus(line));
    }
}

std::pair<std::pair<std::string, int>, std::string_view>
Input::ParseAdjacent(std::string_view line)
{
    auto distance {ParseInt(LeftStrip(line), ' ')};
    auto to = ParseString(distance.second, ' ');
    auto stopname = ParseString(to.second, ',');
    return {{stopname.first, distance.first}, stopname.second};
}

StopData Input::ParseStop(std::string_view line) const {
    auto query {ParseString(line, ' ')};
    auto name {ParseString(query.second, ':')};
    auto c1 {ParseDouble(name.second, ',')};
    auto c2 {ParseDouble(c1.second, ',')};

    std::unordered_map<std::string, int> adjacent;
    std::string_view tail = c2.second;
    while (tail.size() != 0 ) {
        auto adj_enrty = ParseAdjacent(tail);
        adjacent.emplace(adj_enrty.first);
        tail = adj_enrty.second;
    }
    return {name.first, Coordinates{c1.first, c2.first}, adjacent};
}

BusData Input::ParseBus(std::string_view line) const {
    std::vector<std::string> stops;
    auto query_tail {ParseString(line, ' ')};
    auto name_tail {ParseString(query_tail.second, ':')};
    std::string_view tail {name_tail.second};
    bool is_line_bus {tail.find('-') != std::string::npos};
    size_t first_pos {0};
    size_t pos {0};
    while ((pos = tail.find_first_of(m_stop_delimiters, first_pos)) != std::string::npos) {
        stops.emplace_back(Strip(tail.substr(first_pos, pos - first_pos)));
        first_pos = pos + 1;
    }
    stops.emplace_back(Strip(tail.substr(first_pos, tail.size() - first_pos)));

    if (is_line_bus) {
        std::vector<std::string> as;
        as.reserve(stops.size() * 2 - 1);
        as.insert(as.begin(), stops.begin(), stops.end());
        std::move(stops.rbegin() + 1, stops.rend(), std::back_inserter(as));
        stops = std::move(as);
    }
    return {name_tail.first, stops};
}

}
