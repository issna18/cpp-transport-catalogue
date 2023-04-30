#pragma once

#include "domain.h"
#include "json.h"
#include "transport_catalogue.h"

#include <istream>
#include <vector>

namespace json {

Node ToJSON(const BusInfo& info);
Node ToJSON(const StopInfo& info);
Node ToJSON(int request_id, const MapInfo& info);

StopData StopDataFromJSON(const Node& node);
BusData BusDataFromJSON(const Node& node);
RenderSettings GetSettingsFromJSON(const Node& node);

class Reader
{
public:
    Reader(std::istream& in);
    const Node& GetRenderSettings() const;
    const Node& GetStatRequests() const;
    const std::pair<std::vector<StopData>, std::vector<BusData>> GetStopsAndBuses() const;
    const std::vector<Request> GetRequests() const;

private:
    const Node& GetBaseRequests() const;

    json::Document m_json;
    json::Node empty {};
};

} // namespace json
