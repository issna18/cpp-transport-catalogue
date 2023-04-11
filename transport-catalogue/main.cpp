#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string_view>

int main() {
    using namespace std::string_view_literals;

    TransportCatalogue tc;
    {
        Reader::Input ir(std::cin);
        ir.FillCatalogue(tc);
    }

    {
        Reader::Stat sr(std::cin);
        for (auto& [is_bus, name] : sr.GetQueries()){
            if (is_bus) {
                TransportCatalogue::BusInfo info {tc.GetBusInfo(name)};
                std::cout << "Bus "sv << name << ": "sv;
                if (info.status == TransportCatalogue::ResultStatus::NotFound) {
                    std::cout << "not found"sv << std::endl;
                    continue;
                }
                std::cout << info.num_stops << " stops on route, "sv
                          << info.num_unique << " unique stops, "sv
                          << info.route_length << " route length, "sv
                          << std::setprecision(6) << info.route_length/info.geo_length << " curvature"sv
                          << std::endl;

            } else {
                TransportCatalogue::StopInfo info {tc.GetStopInfo(name)};

                std::cout << "Stop "sv << name << ": "sv;

                if (info.status == TransportCatalogue::ResultStatus::NotFound) {
                    std::cout << "not found"sv << std::endl;
                    continue;
                }

                if (info.buses.size() == 0) {
                    std::cout << "no buses"sv << std::endl;
                    continue;
                }

                std::cout << "buses "sv << info.buses << std::endl;
            }
        }
    }

return 0;
}
