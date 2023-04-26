#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <iostream>

int main() {
    json::Reader input(std::cin);

    MapRenderer renderer;
    renderer.SetSettings(json::GetSettingsFromJSON(input.GetRenderSettings()));

    RequestHandler request_handler;
    request_handler.ProcessBaseRequests(input);
    request_handler.ProcessStatRequests(input);

    renderer.Draw(request_handler.GetMap());

    return 0;
}
