#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <istream>


namespace json {

Node ToJSON(const TransportCatalogue::BusInfo& info);
Node ToJSON(const TransportCatalogue::StopInfo& info);
StopData StopDataFromJSON(const json::Node& node);
BusData BusDataFromJSON(const json::Node& node);

class Reader
{
public:
    Reader(std::istream& in);
    const Node& GetBaseRequests() const;
    const Node& GetStatRequests() const;

private:
    json::Document m_json;
};
}
