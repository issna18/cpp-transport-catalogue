#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <string_view>
#include <unordered_map>

namespace transport {

class Router
{
public:
    Router(const TransportCatalogue& catalogue, const RoutingSettings& settings);

    std::unique_ptr<Info> BuildRoute(std::string_view from,
                                     std::string_view to) const;

private:
    struct EdgeData {
        std::string_view bus;
        size_t span_count {0};
        bool is_wait {false};
    };

    void BuildVertices(const std::deque<Stop>& stops);

    void BuildEdges(const std::deque<Bus>& buses);

    void BuildEdgesForBus(const Bus& bus);

    template<class Iterator>
    void BuildEdgesForBusStops(Iterator begin, Iterator end, std::string_view bus) {
        for (Iterator it {begin}; it != end; it++) {
            double distance {0.0};
            for (Iterator jt {std::next(it)}; jt != end; jt++) {
                distance += m_transport_catalogue.GetDistance((*std::prev(jt))->name, (*jt)->name);
                const auto& from_go {m_name_to_vertex_go.at((*it)->name)};
                const auto& to_wait {m_name_to_vertex_wait.at((*jt)->name)};
                const size_t span_count {static_cast<size_t>(std::distance(it, jt))};
                MakeEdge(from_go, to_wait, CalculateWeight(distance), {bus, span_count});
            }
        }
    }

    inline double CalculateWeight(double distance) const;

    graph::EdgeId MakeEdge(graph::VertexId from,
                           graph::VertexId to,
                           double weight,
                           const EdgeData& data);

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
    std::unique_ptr<Info> Request(const transport::Router& router) const
    {
        return router.BuildRoute(from, to);
    }
};
