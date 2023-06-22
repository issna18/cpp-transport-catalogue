#include "json_builder.h"
#include "request_handler.h"
#include "serialization.h"
#include "transport_router.h"

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

    json::Node ErrorMessage(int request_id) {
        return json::Builder{}
            .StartDict()
                .Key("request_id"s).Value(request_id)
                .Key("error_message"s).Value("not found"s)
            .EndDict()
            .Build();
    }

    json::Node operator()(const BusQuery& query) {
        const auto info {std::get<BusInfo>(query.Get(catalogue))};
        if (info.status == ResultStatus::NotFound) {
            return ErrorMessage(info.request_id);
        }
        return info.ToJSON();
    }

    json::Node operator()(const StopQuery& query) {
        const auto info {std::get<StopInfo>(query.Get(catalogue))};
        if (info.status == ResultStatus::NotFound) {
            return ErrorMessage(info.request_id);
        }
        return info.ToJSON();
    }

    json::Node operator()(const MapQuery& query) {
        const auto info {std::get<MapInfo>(query.Get(renderer))};
        return info.ToJSON();
        //return ErrorMessage(query.request_id);
    }

    json::Node operator()(const RouteQuery& query) {
        const auto info {std::get<RouteInfo>(query.Get(router))};
        if (info.status == ResultStatus::NotFound) {
            return ErrorMessage(info.request_id);
        }
        return info.ToJSON();
    }
};

RequestHandler::RequestHandler(std::istream& in)
    : m_reader(in),
      m_renderer(m_transport_catalogue, m_reader.GetRenderSettings())
{}

void RequestHandler::ProcessBaseRequests() {
    const auto [stops, buses] {m_reader.GetStopsAndBuses()};

    m_transport_catalogue.AddStops(stops);
    m_transport_catalogue.AddBuses(buses);
}

void RequestHandler::ProcessStatRequests(std::ostream& out)
{
    transport::Router router(m_transport_catalogue,
                             json::MakeRoutingSettingsFromJSON(
                                m_reader.GetMainRequest("routing_settings"s))
                            );

    std::vector<Query> queries {m_reader.GetQueries()};

    json::Array results;
    for (const auto& query : queries) {
        results.emplace_back(std::visit(QueryVisitor
                          {m_transport_catalogue,
                           m_renderer,
                           router}, query));
    }

    if (!results.empty()) {
        json::Print(json::Document{std::move(results)}, out);
        out << std::endl;
    }
}

void RequestHandler::Serialize() {
    TransportDatabase database;
    m_transport_catalogue.Serialize(*database.GetData().mutable_catalogue());
    m_renderer.Serialize(*database.GetData().mutable_renderer());

    database.SaveTo(m_reader.GetSerializationSettings().file_name);
}

void RequestHandler::Deserialize() {
    TransportDatabase database;
    database.LoadFrom(m_reader.GetSerializationSettings().file_name);
    m_transport_catalogue.Deserialize(database.GetData().catalogue());
    m_renderer.Deserialize(database.GetData().renderer());
}
