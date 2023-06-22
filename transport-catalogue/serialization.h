#pragma once

#include <transport_catalogue.pb.h>

#include <string>
#include <fstream>

class TransportDatabase
{
public:
    TransportDatabase() {}

    bool SaveTo(const std::string& output_file_name) const {
        std::fstream output_stream(output_file_name, std::ios::out | std::ios::trunc | std::ios::binary);
        return m_proto_database.SerializeToOstream(&output_stream);
    }

    bool LoadFrom(const std::string& input_file_name) {
        std::fstream input_stream(input_file_name, std::ios::in | std::ios::binary);
        return m_proto_database.ParseFromIstream(&input_stream);
    }

    proto::TransportDatabase& GetData() {
        return m_proto_database;
    }

    const proto::TransportDatabase& GetData() const {
        return m_proto_database;
    }

    proto::TransportDatabase m_proto_database;

};

