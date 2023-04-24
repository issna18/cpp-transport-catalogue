#include "json.h"
#include "request_handler.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>

namespace Reader {

Stat::Stat(const TransportCatalogue &transport_catalogue, std::ostream& out)
    : m_transport_catalogue {transport_catalogue},
      m_out {out}
{}

void Stat::Read(const json::Document& jdoc) {
   ProcessRequests(jdoc.GetRoot().AsMap().at("stat_requests"));
}

void Stat::Print(const TransportCatalogue::BusInfo& info) const {
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

void Stat::Print(const TransportCatalogue::StopInfo& info) const {
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

void Stat::ProcessRequests(const json::Node& requests){
    for (const json::Node& req : requests.AsArray()) {
        bool is_bus {"Bus" == req.AsMap().at("type").AsString()};
        int id {req.AsMap().at("id").AsInt()};
        std::string name {req.AsMap().at("name").AsString()};
        if (is_bus) {
            BusQuery query {id, name};
            Print(m_transport_catalogue.GetInfo(query));
        } else {
            StopQuery query {id, name};
            Print(m_transport_catalogue.GetInfo(query));
        }
    }
}

}

