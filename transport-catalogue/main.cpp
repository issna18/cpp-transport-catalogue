#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include "json.h"

#include <iostream>

#include <string_view>

int main() {
    using namespace std::string_view_literals;


    json::Document json_doc {json::Load(std::cin)};

    TransportCatalogue transport_catalogue;
    Reader::Input input_reader(transport_catalogue);
    input_reader.Read(json_doc);

/*
    TransportCatalogue transport_catalogue;
    {
        Reader::Input input_reader(transport_catalogue);
        input_reader.Read(std::cin);
    }

    {
        Reader::Stat stat_reader(transport_catalogue);
        stat_reader.Read(std::cin);
    }
    */

return 0;
}
