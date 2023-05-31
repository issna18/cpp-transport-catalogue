#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <string_view>
#include <unordered_map>

namespace transport {

class Router
{
public:
    struct EdgeData {
        std::string_view bus;
        size_t span_count {0};
        bool is_wait {false};
    };

    Router(const TransportCatalogue& catalogue, const RoutingSettings& settings)
        : m_transport_catalogue {catalogue},
          m_settings {settings}
    {
        const auto& stops {m_transport_catalogue.GetStops()};
        m_graph = std::make_unique<graph::DirectedWeightedGraph<double>>(2 * stops.size());

        BuildVertices(stops);
        BuildEdges(m_transport_catalogue.GetBuses());
        m_router = std::make_unique<graph::Router<double>>(*m_graph);
    };

    void BuildVertices(const std::deque<Stop>& stops) {
        graph::VertexId id {0};
        for (const auto & stop : stops) {
            const auto & wait_vertex {m_vertex_to_name.emplace(id++, stop.name).first};
            /*
            std::cout << "[* w] [" << wait_vertex->second
                      << "](" << wait_vertex->first << ")"
                      << std::endl;
                      */
            m_name_to_vertex_wait.emplace(wait_vertex->second, wait_vertex->first);
            const auto & go_vertex {m_vertex_to_name.emplace(id++, stop.name).first};
            /*
            std::cout << "[* g] [" << go_vertex->second
                      << "](" << go_vertex->first << ")"
                      << std::endl;
                      */
            m_name_to_vertex_go.emplace(go_vertex->second, go_vertex->first);
        }
    }

    void BuildEdges(const std::deque<Bus>& buses) {
        for (const auto & bus : buses) {
            BuildEdgesForBus(bus);
        }
    }

    void BuildEdgesForBus(const Bus& bus) {
        const auto& stops {bus.stops};
        std::cout << "bus " << bus.name << " " << bus.is_roundtrip << std::endl;
        auto first = stops.cbegin();
        auto last = stops.cend();
        if (bus.is_roundtrip) {
            last = std::prev(last);
        } else {
            last = std::prev(last, stops.size() / 2);
        }

        for (auto it = first; it != last; it++) {
            const auto& from_wait {m_name_to_vertex_wait.at((*it)->name)};
            const auto& to_go {m_name_to_vertex_go.at((*it)->name)};
            MakeEdge(from_wait, to_go, m_settings.bus_wait_time, {bus.name, 0, true});
        }

        BuildEdgesForBusStops(first, last, bus.name);
        if (bus.is_roundtrip) return;
        BuildEdgesForBusStops(std::next(first, stops.size() / 2), stops.cend(), bus.name);
    }


    template<class Iterator>
    void BuildEdgesForBusStops(Iterator begin, Iterator end, std::string_view bus) {
        for (Iterator it = begin; it != end; it++) {
            double distance {0.0};
            for (Iterator jt {std::next(it)}; jt != end; jt++) {
                distance += m_transport_catalogue.GetDistance((*std::prev(jt))->name, (*jt)->name);
                const auto& from_go {m_name_to_vertex_go.at((*it)->name)};
                const auto& to_go {m_name_to_vertex_wait.at((*jt)->name)};
                size_t span_count = static_cast<size_t>(std::distance(it, jt));
                MakeEdge(from_go, to_go, CalculateWeight(distance), {bus, span_count});
            }
        }
    }

    double CalculateWeight(double distance) const {
        return (60 * distance) / (1000 * m_settings.bus_velocity);
    }

    graph::EdgeId MakeEdge(graph::VertexId from,
                           graph::VertexId to,
                           double weight,
                           const EdgeData& data)
    {
        graph::EdgeId id = m_graph->AddEdge({
                        from,
                        to,
                        static_cast<double>(weight)
                    });
        /*
        std::cout << "[-] " << id
                  << ": [" << m_vertex_to_name.at(from)
                  << "](" << from
                  << ") => [" << m_vertex_to_name.at(to)
                  << "](" << to
                  << ") = " << weight << std::endl;
                  */
        m_edge_to_data[id] = data;
        return id;
    }

    RouteInfo BuildRoute(std::string_view from,
                         std::string_view to, int request_id) const
    {
        const graph::VertexId& vertex_from {m_name_to_vertex_wait.at(from)};
        const graph::VertexId& vertex_to {m_name_to_vertex_wait.at(to)};
        auto route_info = m_router->BuildRoute(vertex_from, vertex_to);
        if (route_info.has_value()) {
            std::vector<RouteItem> items;
            items.reserve(route_info->edges.size());
            /*
            std::cout << "[" << from << "]=>[" << to << "] :"
                      << route_info->edges.size() << std::endl;
                      */
            for (const auto& id : route_info->edges) {
                const auto& edge = m_graph->GetEdge(id);
                const auto& data = m_edge_to_data.at(id);
                if (data.is_wait) {
                    const auto name = m_vertex_to_name.at(edge.from);
                    items.emplace_back(RouteItem{name, data.is_wait, edge.weight});
                } else {
                    items.emplace_back(RouteItem{data.bus, data.is_wait, edge.weight, data.span_count});
                }

                /*
                std::cout << "\tBus: " << data.bus
                          << ", span_count: " << data.span_count
                          << ", from: " << edge.from
                          << ", to: " << edge.to
                          << ", time: " << edge.weight
                          << std::endl;
                          */
            }
            return {request_id, ResultStatus::Success, route_info->weight, std::move(items)};
        }
        return {request_id};
    }

    const TransportCatalogue& m_transport_catalogue;
    RoutingSettings m_settings;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> m_graph {nullptr};
    std::unique_ptr<graph::Router<double>> m_router {nullptr};
    std::unordered_map<std::string_view, graph::VertexId> m_name_to_vertex_wait;
    std::unordered_map<std::string_view, graph::VertexId> m_name_to_vertex_go;
    std::unordered_map<graph::VertexId, std::string_view> m_vertex_to_name;
    std::unordered_map<graph::EdgeId, EdgeData> m_edge_to_data;
};

}

struct RouteQuery {
    int request_id;
    std::string from;
    std::string to;
    Info Get(const transport::Router& router) const
    {
        return router.BuildRoute(from, to, request_id);
    }
};
