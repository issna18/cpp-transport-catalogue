#include "serialization.h"

bool TransportDatabase::SaveTo(const std::string& output_file_name) const {
    std::fstream output_stream(output_file_name, std::ios::out | std::ios::trunc | std::ios::binary);
    return m_proto_database.SerializeToOstream(&output_stream);
}

bool TransportDatabase::LoadFrom(const std::string& input_file_name) {
    std::fstream input_stream(input_file_name, std::ios::in | std::ios::binary);
    return m_proto_database.ParseFromIstream(&input_stream);
}

proto::TransportDatabase& TransportDatabase::GetData() {
    return m_proto_database;
}

const proto::TransportDatabase& TransportDatabase::GetData() const {
    return m_proto_database;
}
