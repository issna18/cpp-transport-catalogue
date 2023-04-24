#pragma once
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace Reader {

class Stat
{
public:
    Stat(TransportCatalogue& transport_catalogue, std::ostream& out = std::cout);
    void Read(std::istream& input);

private:
    void ProcessLine(std::string_view line);
    void PrintBus(const TransportCatalogue::BusInfo& info) const;
    void PrintStop(const TransportCatalogue::StopInfo& info) const;
    TransportCatalogue& m_transport_catalogue;
    std::ostream& m_out;
};
}
