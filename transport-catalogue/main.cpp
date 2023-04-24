#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>

int main() {
    json::Document json_doc {json::Load(std::cin)};

    TransportCatalogue transport_catalogue;

    {
        Reader::Input input_reader(transport_catalogue);
        input_reader.Read(json_doc);
    }

    {
        Reader::Stat stat_reader(transport_catalogue);
        stat_reader.Read(json_doc);
    }

return 0;
}
