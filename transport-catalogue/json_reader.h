#pragma once

#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <istream>
#include <string_view>
#include <vector>

using Query = std::variant<BusQuery, StopQuery, MapQuery, RouteQuery>;

namespace json {

class Reader
{
public:
    Reader(std::istream& in);
    const Node& GetNodeByKey(const std::string& key) const;
    std::pair<std::vector<StopData>, std::vector<BusData>> GetStopsAndBuses() const;
    std::vector<Query> GetQueries() const;
    RenderSettings GetRenderSettings() const;
    SerializationSettings GetSerializationSettings() const;
    RoutingSettings GetRoutingSettings() const;

private:
    json::Document m_json;
    json::Node empty {};
};

} // namespace json
