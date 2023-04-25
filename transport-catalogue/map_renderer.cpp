#include "map_renderer.h"

#include <map>

namespace sphere {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

} //namespace sphere

void MapRenderer::SetSettings(const RenderSettings& settings) {
    m_settings = settings;

/*
    std::cout << m_settings.width << " "
              << m_settings.height << " "
              << m_settings.padding << " "
              << m_settings.underlayer_color
              << std::endl;
*/
}

void MapRenderer::Draw(const std::deque<Bus>& buses, std::ostream& out)
{
    svg::Document svg;
    std::vector<geo::Coordinates> all_coordinates;
    std::map<const std::string_view, const std::vector<StopPtrConst>> buses_stops;

    for (const auto& bus : buses) {
        buses_stops.emplace(bus.name, bus.stops);
        for (StopPtrConst stop : bus.stops){
            all_coordinates.push_back(stop->coord);
        }
    }

    sphere::Projector projector(all_coordinates.begin(),
                                all_coordinates.end(),
                                m_settings.width, m_settings.height,
                                m_settings.padding);

    size_t palette_idx {0};
    for (const auto& entry : buses_stops) {
        svg.Add(MakeRoute(entry.second, projector, m_settings.color_palette.at(palette_idx)));
        ++palette_idx;
        if (palette_idx == m_settings.color_palette.size()) {
            palette_idx = 0;
        }
    }

    svg.Render(out);
}

svg::Polyline MapRenderer::MakeRoute(const std::vector<StopPtrConst>& stops,
                                     const sphere::Projector& projector,
                                     const svg::Color& color)
{
    svg::Polyline route;
    route.SetFillColor({})
            .SetStrokeColor(color)
            .SetStrokeWidth(static_cast<int>(m_settings.line_width))
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (const auto& stop : stops) {
        route.AddPoint(projector(stop->coord));
    }
    return route;
}
