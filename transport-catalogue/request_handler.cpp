#include "request_handler.h"
#include "serialization.h"

#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <variant>

using namespace std::string_literals;

struct QueryVisitor {
    const TransportCatalogue& catalogue;
    const MapRenderer& renderer;
    const transport::Router& router;

    json::Node operator()(const BusQuery& query) {
        return query.Request(catalogue).get()->ToJSON(query.request_id);
    }

    json::Node operator()(const StopQuery& query) {
        return query.Request(catalogue).get()->ToJSON(query.request_id);
    }

    json::Node operator()(const MapQuery& query) {
        return query.Request(renderer).get()->ToJSON(query.request_id);
    }

    json::Node operator()(const RouteQuery& query) {
        return query.Request(router).get()->ToJSON(query.request_id);
    }
};

RequestHandler::RequestHandler(std::istream& in)
    : m_reader(in),
      m_renderer(m_transport_catalogue, m_reader.GetRenderSettings()),
      m_router(m_transport_catalogue, m_reader.GetRoutingSettings())
{}

void RequestHandler::ProcessBaseRequests()
{
    const auto [stops, buses] {m_reader.GetStopsAndBuses()};

    m_transport_catalogue.AddStops(stops);
    m_transport_catalogue.AddBuses(buses);
    m_router.BuildGraph();
}

void RequestHandler::ProcessStatRequests(std::ostream& out)
{
    std::vector<Query> queries {m_reader.GetQueries()};

    json::Array results;
    for (const auto& query : queries) {
        results.emplace_back(std::visit(QueryVisitor
                          {m_transport_catalogue,
                           m_renderer,
                           m_router}, query));
    }

    if (!results.empty()) {
        json::Print(json::Document{std::move(results)}, out);
        out << std::endl;
    }
}

void RequestHandler::Serialize() const
{
    TransportDatabase database;
    m_transport_catalogue.Serialize(*database.GetData().mutable_catalogue());
    m_renderer.Serialize(*database.GetData().mutable_renderer());
    m_router.Serialize(*database.GetData().mutable_router());
    database.SaveTo(m_reader.GetSerializationSettings().file_name);
}

void RequestHandler::Deserialize()
{
    TransportDatabase database;
    database.LoadFrom(m_reader.GetSerializationSettings().file_name);
    m_transport_catalogue.Deserialize(database.GetData().catalogue());
    m_renderer.Deserialize(database.GetData().renderer());
    m_router.Deserialize(database.GetData().router());
}
