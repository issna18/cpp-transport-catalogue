#include "json_reader.h"
#include "request_handler.h"

#include <iostream>

int main() {
    json::Reader input(std::cin);

    RequestHandler request_handler;
    request_handler.ProcessBaseRequests(input);
    request_handler.ProcessStatRequests(input);

    return 0;
}
