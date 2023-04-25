#pragma once

#include "domain.h"
#include "json.h"
#include "transport_catalogue.h"

#include <istream>

namespace json {

Node ToJSON(const TransportCatalogue::BusInfo& info);
Node ToJSON(const TransportCatalogue::StopInfo& info);
StopData StopDataFromJSON(const Node& node);
BusData BusDataFromJSON(const Node& node);
RenderSettings GetSettingsFromJSON(const Node& node);

class Reader
{
public:
    Reader(std::istream& in);
    const Node& GetBaseRequests() const;
    const Node& GetStatRequests() const;
    const Node& GetRenderSettings() const;

private:
    json::Document m_json;
};

} // namespace json
