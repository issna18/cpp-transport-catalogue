#include "map_renderer.h"

#include <set>
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
    std::map<const std::string_view, BusPtrConst> buses_stops;
    std::map<const std::string_view, geo::Coordinates> stops_coord;

    for (const auto& bus : buses) {
        buses_stops.emplace(bus.name, &bus);
        for (StopPtrConst stop : bus.stops){
            all_coordinates.push_back(stop->coord);
            if (stops_coord.count(stop->name) == 0) stops_coord.emplace(stop->name, stop->coord);
        }
    }

    sphere::Projector projector(all_coordinates.begin(),
                                all_coordinates.end(),
                                m_settings.width, m_settings.height,
                                m_settings.padding);

    std::vector<svg::Polyline> routes_layer;
    routes_layer.reserve(buses_stops.size());
    std::vector<svg::Text> bus_layer;

    size_t bus_counter {0};
    for (const auto& [name, bus] : buses_stops) {
        const svg::Color& color {
            m_settings.color_palette.at(bus_counter++ % m_settings.color_palette.size())
        };
        const auto& stops {bus->stops};

        routes_layer.emplace_back(MakeRoute(stops, projector, color));

        std::vector<StopPtrConst> end_stops;

        if(bus->is_roundtrip) {
            end_stops.push_back(stops.front());
        } else {
            end_stops.push_back(stops.front());
            end_stops.push_back(stops.at(stops.size() / 2));
        }

        const auto& coord1 {end_stops.front()->coord};
        bus_layer.emplace_back(MakeBgBusLabel(name, projector(coord1)));
        bus_layer.emplace_back(MakeBusLabel(name, projector(coord1),color));

        if (!bus->is_roundtrip && end_stops.front() != end_stops.back()) {
            const auto& coord2 {end_stops.back()->coord};
            bus_layer.push_back(MakeBgBusLabel(name, projector(coord2)));
            bus_layer.push_back(MakeBusLabel(name, projector(coord2),color));
        }
    }

    std::vector<svg::Circle> circle_layer;
    std::vector<svg::Text> stop_layer;

    for (const auto& [stop, coord] : stops_coord) {
        svg::Circle circle;
        circle.SetCenter(projector(coord))
                .SetRadius(m_settings.stop_radius)
                .SetFillColor("white");
        circle_layer.push_back(circle);

        stop_layer.emplace_back(MakeBgStopLabel(stop, projector(coord)));
        stop_layer.emplace_back(MakeStopLabel(stop, projector(coord),"black"));
    }

    for(auto&& item : routes_layer) {
        svg.Add(std::move(item));
    }

    for(auto&& item : bus_layer) {
        svg.Add(std::move(item));
    }

    for(auto&& item : circle_layer) {
        svg.Add(std::move(item));
    }

    for(auto&& item : stop_layer) {
        svg.Add(std::move(item));
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
        .SetStrokeWidth(m_settings.line_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (const auto& stop : stops) {
        route.AddPoint(projector(stop->coord));
    }
    return route;
}

svg::Text MapRenderer::MakeBusLabel(std::string_view text,
                                    const svg::Point& point,
                                    const svg::Color& color) {
    /*
    x и y — координаты соответствующей остановки;
    смещение dx и dy равно настройке bus_label_offset;
    размер шрифта font-size равен настройке bus_label_font_size;
    название шрифта font-family — "Verdana";
    толщина шрифта font-weight — "bold".
    содержимое — название автобуса.
    */
    svg::Text label;
    label.SetFillColor(color)
            .SetPosition(point)
            .SetOffset(m_settings.bus_label_offset)
            .SetFontSize(m_settings.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(std::string(text));
    return label;
}

svg::Text MapRenderer::MakeBgBusLabel(std::string_view text, const svg::Point &point){
    svg::Text label {MakeBusLabel(text, point, m_settings.underlayer_color)};
    label.SetStrokeColor(m_settings.underlayer_color)
         .SetStrokeWidth(m_settings.underlayer_width)
         .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
         .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return label;
}


svg::Text MapRenderer::MakeStopLabel(std::string_view text,
                                     const svg::Point &point,
                                     const svg::Color& color)
{
    svg::Text label;
    label.SetPosition(point)
         .SetOffset(m_settings.stop_label_offset)
         .SetFontSize(m_settings.stop_label_font_size)
         .SetFontFamily("Verdana")
         .SetData(std::string(text))
         .SetFillColor(color);
    return label;
}

svg::Text MapRenderer::MakeBgStopLabel(std::string_view text,
                                       const svg::Point &point)
{
    svg::Text label {MakeStopLabel(text, point, m_settings.underlayer_color)};
    label.SetStrokeColor(m_settings.underlayer_color)
         .SetStrokeWidth(m_settings.underlayer_width)
         .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
         .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return label;
}
