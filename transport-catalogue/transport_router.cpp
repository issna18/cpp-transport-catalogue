#include "transport_router.h"

#include <string_view>

namespace transport {

Router::Router(const TransportCatalogue& catalogue,
               const RoutingSettings& settings)
    : m_transport_catalogue {catalogue},
      m_settings {settings}
{ }

void Router::BuildGraph() {
    const auto& stops {m_transport_catalogue.GetStops()};
    m_graph = std::make_unique<graph::DirectedWeightedGraph<double>>(2 * stops.size());
    BuildVertices(stops);
    BuildEdges(m_transport_catalogue.GetBuses());
    m_router = std::make_unique<graph::Router<double>>(*m_graph);
}

std::unique_ptr<Info>
Router::BuildRoute(std::string_view from,
                   std::string_view to) const
{
    const graph::VertexId& vertex_from {m_name_to_vertex_wait.at(from)};
    const graph::VertexId& vertex_to {m_name_to_vertex_wait.at(to)};

    auto route_info = m_router->BuildRoute(vertex_from, vertex_to);
    if (route_info.has_value()) {
        std::vector<RouteInfo::RouteItem> items;
        items.reserve(route_info->edges.size());
        for (const auto& edge_id : route_info->edges) {
            items.emplace_back([this](graph::EdgeId id) {
                const auto& edge {m_graph->GetEdge(id)};
                const auto& data {m_edge_to_data.at(id)};
                if (data.is_wait) {
                    return RouteInfo::RouteItem{m_vertex_to_name.at(edge.from), data.is_wait, edge.weight};
                }
                return RouteInfo::RouteItem{data.bus, data.is_wait, edge.weight, data.span_count};
            } (edge_id) );
        }
        return std::make_unique<RouteInfo>(route_info->weight,
                                           std::move(items));
    }
    return std::make_unique<ErrorInfo>();
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
    constexpr int meters_in_kilometer {1000};
    constexpr int minutes_in_hour {60};
    return (minutes_in_hour * distance) /
           (meters_in_kilometer * m_settings.bus_velocity);
}

graph::EdgeId Router::MakeEdge(graph::VertexId from,
                               graph::VertexId to,
                               double weight,
                               const EdgeData& data) {
    graph::EdgeId id = m_graph->AddEdge({from, to, weight});

    m_edge_to_data[id] = data;
    return id;
}

bool Router::Serialize(proto::transport::Router& proto_router) const {
    auto proto_settings = proto_router.mutable_settings();
    proto_settings->set_bus_wait_time(m_settings.bus_wait_time);
    proto_settings->set_bus_velocity(m_settings.bus_velocity);

    auto proto_graph = proto_router.mutable_graph();
    m_graph->Serialise(*proto_graph);

    auto proto_vertex_to_name = proto_router.mutable_vertex_to_name();
    for(const auto& [id, name] : m_vertex_to_name) {
        (*proto_vertex_to_name)[id] = std::string(name);
    }

    auto proto_name_to_vertex_wait = proto_router.mutable_name_to_vertex_wait();
    for(const auto& [name, id] : m_name_to_vertex_wait) {
        (*proto_name_to_vertex_wait)[std::string(name)] = id;
    }

    auto proto_name_to_vertex_go = proto_router.mutable_name_to_vertex_go();
    for(const auto& [name, id] : m_name_to_vertex_go) {
        (*proto_name_to_vertex_go)[std::string(name)] = id;
    }

    auto proto_edge_to_data = proto_router.mutable_edge_to_data();
    for(const auto& [edge, data] : m_edge_to_data) {
        auto& proto_data = (*proto_edge_to_data)[edge] = {};
        proto_data.set_bus(std::string(data.bus));
        proto_data.set_span_count(data.span_count);
        proto_data.set_is_wait(data.is_wait);
    }

    return true;
}

bool Router::Deserialize(const proto::transport::Router &proto_router) {

    m_settings.bus_wait_time = proto_router.settings().bus_wait_time();
    m_settings.bus_velocity = proto_router.settings().bus_velocity();

    const auto nodes_count {m_transport_catalogue.GetStops().size()};
    m_graph = std::make_unique<graph::DirectedWeightedGraph<double>>(2 * nodes_count);
    m_graph->Deserialise(proto_router.graph());
    m_router = std::make_unique<graph::Router<double>>(*m_graph);

    for(const auto& [vertex, name] : proto_router.vertex_to_name()) {
        m_vertex_to_name[vertex] = m_transport_catalogue.GetStop(name)->name;
    }

    for(const auto& [name, vertex] : proto_router.name_to_vertex_wait()) {
        m_name_to_vertex_wait[m_transport_catalogue.GetStop(name)->name] = vertex;
    }

    for(const auto& [name, vertex] : proto_router.name_to_vertex_go()) {
        m_name_to_vertex_go[m_transport_catalogue.GetStop(name)->name] = vertex;
    }

    for(const auto& [edge, data] : proto_router.edge_to_data()) {
        m_edge_to_data[edge] =
                EdgeData {
                    m_transport_catalogue.GetBus(data.bus())->name,
                    data.span_count(),
                    data.is_wait()
                };
    }

    return true;
}

} //namespace transport




