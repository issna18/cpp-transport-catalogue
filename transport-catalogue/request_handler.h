#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <ostream>

namespace Reader {

class Stat
{
public:
    Stat(const TransportCatalogue& transport_catalogue, std::ostream& out = std::cout);
    void Read(const json::Document& jdoc);

private:
    void ProcessRequests(const json::Node& requests);
    void Print(const TransportCatalogue::BusInfo& info) const;
    void Print(const TransportCatalogue::StopInfo& info) const;
    const TransportCatalogue& m_transport_catalogue;
    std::ostream& m_out;
};
}
