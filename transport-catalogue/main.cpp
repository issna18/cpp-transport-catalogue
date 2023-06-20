#include "request_handler.h"

#include <iostream>
#include <string_view>

using namespace std::string_view_literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);
    RequestHandler request_handler(std::cin);

    if (mode == "make_base"sv) {
        request_handler.ProcessBaseRequests();
        request_handler.Serialize();
    } else if (mode == "process_requests"sv) {
        request_handler.Deserialize();
        request_handler.ProcessStatRequests();
    } else {
        PrintUsage();
        return 1;
    }
}
