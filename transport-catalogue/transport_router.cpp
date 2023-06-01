#include "transport_router.h"

#include <string_view>

namespace transport {

Router::Router(const TransportCatalogue& catalogue, const RoutingSettings& settings)
    : m_transport_catalogue {catalogue},
      m_settings {settings}
{
    const auto& stops {m_transport_catalogue.GetStops()};
    m_graph = std::make_unique<graph::DirectedWeightedGraph<double>>(2 * stops.size());

    BuildVertices(stops);
    BuildEdges(m_transport_catalogue.GetBuses());
    m_router = std::make_unique<graph::Router<double>>(*m_graph);
}

Info Router::BuildRoute(std::string_view from,
                         std::string_view to, int request_id) const {
        const graph::VertexId& vertex_from {m_name_to_vertex_wait.at(from)};
        const graph::VertexId& vertex_to {m_name_to_vertex_wait.at(to)};
        auto route_info = m_router->BuildRoute(vertex_from, vertex_to);
        if (route_info.has_value()) {
            std::vector<RouteItem> items;
            items.reserve(route_info->edges.size());
            for (const auto& edge_id : route_info->edges) {

                items.emplace_back([this](graph::EdgeId id){
                    const auto& edge {m_graph->GetEdge(id)};
                    const auto& data {m_edge_to_data.at(id)};
                    if (data.is_wait)
                        return RouteItem{m_vertex_to_name.at(edge.from), data.is_wait, edge.weight};
                    return RouteItem{data.bus, data.is_wait, edge.weight, data.span_count};
                    }(edge_id)
                );
            }
            return RouteInfo{request_id, ResultStatus::Success, route_info->weight, std::move(items)};
        }
        return RouteInfo{request_id};
    }

void Router::BuildVertices(const std::deque<Stop>& stops) {
    graph::VertexId id {0};
    for (const auto & stop : stops) {
        const auto & wait_vertex {m_vertex_to_name.emplace(id++, stop.name).first};
        m_name_to_vertex_wait.emplace(wait_vertex->second, wait_vertex->first);
        const auto & go_vertex {m_vertex_to_name.emplace(id++, stop.name).first};
        m_name_to_vertex_go.emplace(go_vertex->second, go_vertex->first);
    }
}

void Router::BuildEdges(const std::deque<Bus>& buses) {
    for (const auto & bus : buses) {
        BuildEdgesForBus(bus);
    }
}

void Router::BuildEdgesForBus(const Bus& bus) {
    const auto& stops {bus.stops};
    const auto first {stops.cbegin()};
    const auto last = [&stops](bool is_roundtrip){
        if (is_roundtrip) return std::prev(stops.cend());
        return std::prev(stops.cend(), stops.size() / 2);
    }(bus.is_roundtrip);

    for (auto it {first}; it != last; it++) {
        const auto& from_wait {m_name_to_vertex_wait.at((*it)->name)};
        const auto& to_go {m_name_to_vertex_go.at((*it)->name)};
        MakeEdge(from_wait, to_go, m_settings.bus_wait_time, {bus.name, 0, true});
    }

    BuildEdgesForBusStops(first, stops.cend(), bus.name);
    if (bus.is_roundtrip) return;
    BuildEdgesForBusStops(std::next(first, stops.size() / 2), stops.cend(), bus.name);
}

inline double Router::CalculateWeight(double distance) const {
    return (60 * distance) / (1000 * m_settings.bus_velocity);
}

graph::EdgeId Router::MakeEdge(graph::VertexId from,
                               graph::VertexId to,
                               double weight,
                               const EdgeData& data) {
    graph::EdgeId id = m_graph->AddEdge({from, to, weight});

    m_edge_to_data[id] = data;
    return id;
}

} //namespace transport




