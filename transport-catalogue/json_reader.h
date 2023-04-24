#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace Reader {

class Input
{
public:
    Input(TransportCatalogue& transport_cataloge);
    void Read(const json::Document& jdoc);

private:
    void ProcessBaseRequests(const json::Node& requests);
    StopData ParseStop(const json::Node& node) const;
    BusData ParseBus(const json::Node& node) const;
    void FillCatalogue() const;

    std::vector<StopData> m_stops;
    std::vector<BusData> m_buses;
    TransportCatalogue& m_transport_cataloge;
};
}
