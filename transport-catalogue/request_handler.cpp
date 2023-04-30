#include "request_handler.h"
#include "map_renderer.h"

#include <string>
#include <sstream>
#include <vector>
#include <variant>

using namespace std::string_literals;

struct QueryVisitor {
    const TransportCatalogue& catalogue;
    const MapRenderer& renderer;
    json::Node operator()(const BusQuery& query) { return json::ToJSON(std::get<BusInfo>(query.Get(catalogue))); }
    json::Node operator()(const StopQuery& query) { return json::ToJSON(std::get<StopInfo>(query.Get(catalogue))); }
    json::Node operator()(const MapQuery& query) {
        const auto info {std::get<MapInfo>(query.Get(catalogue))};
        std::stringstream ssout;
        renderer.Draw(info.buses, ssout);
        return json::Dict{
            {"request_id"s, json::Node(info.request_id)},
            {"map"s, json::Node(ssout.str())},
        };
    }
};

RequestHandler::RequestHandler()
{}

void RequestHandler::ProcessBaseRequests(const json::Reader& reader) {
    const auto [stops, buses] {reader.GetStopsAndBuses()};

    m_transport_catalogue.AddStops(stops);
    m_transport_catalogue.AddBuses(buses);
}

void RequestHandler::ProcessStatRequests(const json::Reader &reader, std::ostream& out){
    MapRenderer renderer(json::GetSettingsFromJSON(reader.GetRenderSettings()));
    json::Array results;
    std::vector<Request> requests {reader.GetRequests()};

    for (const auto& request : requests) {
        results.emplace_back(std::visit(QueryVisitor {m_transport_catalogue, renderer}, request));
    }

    if (!results.empty()) {
        json::PrintNode(json::Node{std::move(results)}, out);
        out << std::endl;
    }
}
