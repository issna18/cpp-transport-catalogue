#pragma once

#include "graph.h"
#include <transport_catalogue.pb.h>

#include <string>
#include <fstream>

class TransportDatabase
{
public:
    TransportDatabase() {}

    bool SaveTo(const std::string& output_file_name) const;
    bool LoadFrom(const std::string& input_file_name);
    proto::TransportDatabase& GetData();
    const proto::TransportDatabase& GetData() const;

private:
    proto::TransportDatabase m_proto_database;

};

namespace graph {
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

} //namespace graph

