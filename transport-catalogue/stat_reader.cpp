#include "json.h"
#include "stat_reader.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>

namespace Reader {

Stat::Stat(TransportCatalogue& transport_catalogue, std::ostream& out)
    : m_transport_catalogue {transport_catalogue},
      m_out {out}
{}

void Stat::Read(std::istream& input) {
    /*
    std::string str;
    int num_queries;
    input >> num_queries;
    getline(input,str);

    for (int i {0}; i < num_queries; ++i) {
        std::getline(input, str);
        str = Strip(str);
        if (str.empty()) continue;
        ProcessLine(str);
    }
    */
}

void Stat::PrintBus(const TransportCatalogue::BusInfo& info) const {
 using namespace std::string_view_literals;
    m_out << "Bus "sv << info.name << ": "sv;
    if (info.status == TransportCatalogue::ResultStatus::NotFound) {
        m_out << "not found"sv << std::endl;
        return;
    }
    m_out << info.num_stops << " stops on route, "sv
              << info.num_unique << " unique stops, "sv
              << info.route_length << " route length, "sv
              << std::setprecision(6) << info.route_length/info.geo_length << " curvature"sv
              << std::endl;
}

void Stat::PrintStop(const TransportCatalogue::StopInfo& info) const {
    using namespace std::string_view_literals;
    m_out << "Stop "sv << info.name << ": "sv;

    if (info.status == TransportCatalogue::ResultStatus::NotFound) {
        m_out << "not found"sv << std::endl;
        return;
    }

    if (info.buses.size() == 0) {
        m_out << "no buses"sv << std::endl;
        return;
    }

    m_out << "buses "sv << info.buses << std::endl;
}

void Stat::ProcessLine(std::string_view line){
    /*
    auto query {ParseString(line, ' ')};
    bool is_bus {query.first.front() == 'B'};
    auto name {ParseString(query.second, ':')};
    if(is_bus) {
        PrintBus(m_transport_catalogue.GetBusInfo(name.first));
    } else {
        PrintStop(m_transport_catalogue.GetStopInfo(name.first));
    }
    */
}

}

