#pragma once

#include "domain.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <istream>
#include <vector>

using Query = std::variant<BusQuery, StopQuery, MapQuery, RouteQuery>;

namespace json {

//TODO: move into domain.h conrresponding constructors
RenderSettings MakeRenderSettingsFromJSON(const Node& node);
RoutingSettings MakeRoutingSettingsFromJSON(const Node& node);

class Reader
{
public:
    Reader(std::istream& in);
    const Node& GetRenderSettings() const;
    const Node& GetRoutingSettings() const;
    const Node& GetStatRequests() const;
    const std::pair<std::vector<StopData>, std::vector<BusData>> GetStopsAndBuses() const;
    const std::vector<Query> GetQueries() const;

private:
    const Node& GetBaseRequests() const;

    json::Document m_json;
    json::Node empty {};
};

} // namespace json
