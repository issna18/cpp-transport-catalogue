#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>

int main() {
    json::Document json_doc {json::Load(std::cin)};

    TransportCatalogue transport_catalogue;

    {
        json::Reader input_reader(transport_catalogue);
        input_reader.Read(json_doc);
    }

    {
        RequestHandler request_handler(transport_catalogue);
        request_handler.Read(json_doc);
    }

return 0;
}
