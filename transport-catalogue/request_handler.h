#pragma once

#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <iostream>

class RequestHandler
{
public:
    RequestHandler(std::istream& in);
    void ProcessBaseRequests();
    void ProcessStatRequests(std::ostream& out = std::cout);
    void Serialize() const;
    void Deserialize();

private:
    const json::Reader m_reader;
    TransportCatalogue m_transport_catalogue;
    MapRenderer m_renderer;
    transport::Router m_router;
};
