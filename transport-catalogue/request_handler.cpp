#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

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
    }

    json::Node operator()(const RouteQuery& query) {
        const auto info {std::get<RouteInfo>(query.Get(router))};
        if (info.status == ResultStatus::NotFound) {
            return ErrorMessage(info.request_id);
        }
        return info.ToJSON();
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
    MapRenderer renderer(m_transport_catalogue,
                         json::MakeRenderSettingsFromJSON(reader.GetRenderSettings()));
    transport::Router router(m_transport_catalogue,
                             json::MakeRoutingSettingsFromJSON(reader.GetRoutingSettings()));

    std::vector<Query> queries {reader.GetQueries()};

    json::Array results;
    for (const auto& query : queries) {
        results.emplace_back(std::visit(QueryVisitor {m_transport_catalogue, renderer, router}, query));
    }

    if (!results.empty()) {
        json::Print(json::Document{std::move(results)}, out);
        out << std::endl;
    }
}
