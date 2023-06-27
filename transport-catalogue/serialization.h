#pragma once

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

