#pragma once

#include "json_reader.h"
#include "transport_catalogue.h"

#include <ostream>


class RequestHandler
{
public:
    RequestHandler();
    void ProcessBaseRequests(const json::Reader& reader);
    void ProcessStatRequests(const json::Reader& reader, std::ostream& out = std::cout);
    const std::deque<Bus>& GetMap() const;

private:
    TransportCatalogue m_transport_catalogue;
    //std::ostream& m_out;
};
