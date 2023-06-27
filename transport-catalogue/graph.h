#pragma once

#include <graph.pb.h>

#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;
    explicit DirectedWeightedGraph(size_t vertex_count);
    EdgeId AddEdge(const Edge<Weight>& edge);

    size_t GetVertexCount() const;
    size_t GetEdgeCount() const;
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    bool Serialise(proto::graph::Graph &proto_graph) const;
    bool Deserialise(const proto::graph::Graph &proto_graph);

    void Print() const;


private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}

template <typename Weight>
bool DirectedWeightedGraph<Weight>::Serialise(proto::graph::Graph &proto_graph) const {

    for(const auto& edge : edges_) {
        auto proto_edge = proto_graph.add_edges();
        proto_edge->set_from(edge.from);
        proto_edge->set_to(edge.to);
        proto_edge->set_weight(edge.weight);
    }

    for(const auto& list : incidence_lists_) {
        auto proto_list = proto_graph.add_incidence_lists();
        for(const auto& edge : list) {
            proto_list->add_edges_id(edge);
        }
    }
    return true;
}

template <typename Weight>
bool DirectedWeightedGraph<Weight>::Deserialise(const proto::graph::Graph &proto_graph)
{
    for (const auto& proto_edge : proto_graph.edges()) {
        AddEdge({
                    proto_edge.from(),
                    proto_edge.to(),
                    proto_edge.weight()
                });
    }
    return true;
}

}  // namespace graph
