#pragma once

#include "json_reader.h"
#include "transport_catalogue.h"

#include <ostream>

class RequestHandler
{
public:
    RequestHandler(std::istream& in);
    void ProcessBaseRequests();
    void ProcessStatRequests(std::ostream& out = std::cout);
    void Serialize();
    void Deserialize();
private:
    TransportCatalogue m_transport_catalogue;
    const json::Reader m_reader;
};
