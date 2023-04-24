#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <ostream>


class RequestHandler
{
public:
    RequestHandler(const TransportCatalogue& transport_catalogue, std::ostream& out = std::cout);
    void Read(const json::Document& jdoc);

private:
    void ProcessRequests(const json::Node& requests);
    json::Node Print(const TransportCatalogue::BusInfo& info) const;
    json::Node Print(const TransportCatalogue::StopInfo& info) const;
    const TransportCatalogue& m_transport_catalogue;
    std::ostream& m_out;
};

