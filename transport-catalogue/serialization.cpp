#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

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

void RequestHandler::Serialize() const
{
    TransportDatabase database;
    m_transport_catalogue.Serialize(*database.GetData().mutable_catalogue());
    m_renderer.Serialize(*database.GetData().mutable_renderer());
    m_router.Serialize(*database.GetData().mutable_router());
    database.SaveTo(m_reader.GetSerializationSettings().file_name);
}

void RequestHandler::Deserialize()
{
    TransportDatabase database;
    database.LoadFrom(m_reader.GetSerializationSettings().file_name);
    m_transport_catalogue.Deserialize(database.GetData().catalogue());
    m_renderer.Deserialize(database.GetData().renderer());
    m_router.Deserialize(database.GetData().router());
}

bool TransportCatalogue::Serialize(proto::TransportCatalogue& proto_catalogue) const
{
    std::unordered_map<std::string_view, size_t> stops_to_id;
    std::unordered_map<std::string_view, size_t> buses_to_id;

    size_t stop_id {0};
    for (const auto& stop : m_dqstops) {
        auto proto_stop = proto_catalogue.add_stops();
        stops_to_id.emplace(stop.name, stop_id);
        proto_stop->set_id(stop_id++);
        proto_stop->set_name(stop.name);
        proto_stop->set_lat(stop.coord.lat);
        proto_stop->set_lng(stop.coord.lng);
    }

    size_t bus_id {0};
    for (const auto& bus : m_dqbuses) {
        auto proto_bus = proto_catalogue.add_buses();
        buses_to_id.emplace(bus.name, bus_id);
        proto_bus->set_id(bus_id++);
        proto_bus->set_name(bus.name);
        for (const auto& stop : bus.stops) {
            proto_bus->add_stops(stops_to_id.at(stop->name));
        }
        proto_bus->set_is_roundtrip(bus.is_roundtrip);
        proto_bus->set_num_unique(bus.num_unique);
        proto_bus->set_geo_length(bus.geo_length);
        proto_bus->set_route_length(bus.route_length);
    }

    for (const auto& [pair_stop, distance] : m_stops_distance) {
        auto proto_distance = proto_catalogue.add_distances();
        proto_distance->set_stop_first(stops_to_id.at(pair_stop.first));
        proto_distance->set_stop_second(stops_to_id.at(pair_stop.second));
        proto_distance->set_value(distance);
    }

    for (const auto& [stop, set_buses] : m_stop_to_buses) {
        auto proto_stop_to_buses = proto_catalogue.add_stop_to_buses();
        proto_stop_to_buses->set_stop_id(stops_to_id.at(stop));
        for(const auto& bus_name : set_buses) {
            proto_stop_to_buses->add_buses_id(buses_to_id.at(bus_name));
        }
    }

    return true;
}

bool TransportCatalogue::Deserialize(const proto::TransportCatalogue& proto_catalogue)
{
    std::unordered_map<size_t, StopPtrConst> id_to_stop;
    std::unordered_map<size_t, BusPtrConst> id_to_bus;

    for (const auto& proto_stop : proto_catalogue.stops()) {
        StopPtrConst stop_ptr {
            EmplaceStop({proto_stop.name(),
                        {proto_stop.lat(), proto_stop.lng()}
                        })
        };
        id_to_stop.emplace(proto_stop.id(), stop_ptr);
    }

    for (const auto& proto_distance : proto_catalogue.distances()) {
        SetDistance(id_to_stop.at(proto_distance.stop_first())->name,
                    id_to_stop.at(proto_distance.stop_second())->name,
                    proto_distance.value()
                    );
    }

    for (const auto& proto_bus : proto_catalogue.buses()) {
        std::vector<StopPtrConst> stops_ptrs;
        stops_ptrs.reserve(proto_bus.stops_size());

        for (const auto& stop_id : proto_bus.stops()) {
            stops_ptrs.emplace_back(id_to_stop.at(stop_id));
        }

        BusPtrConst bus_ptr {
            EmplaceBus({std::string(proto_bus.name()),
                        std::move(stops_ptrs),
                        proto_bus.num_unique(),
                        proto_bus.route_length(),
                        proto_bus.is_roundtrip()
                       })
        };
        id_to_bus.emplace(proto_bus.id(), bus_ptr);
    }

    for (const auto& proto_stop_to_buses : proto_catalogue.stop_to_buses()) {
        std::set<std::string_view> buses_set;
        for (const auto& bus_id : proto_stop_to_buses.buses_id()) {
            buses_set.insert(id_to_bus.at(bus_id)->name);
        }
        m_stop_to_buses.emplace(
                    id_to_stop.at(proto_stop_to_buses.stop_id())->name,
                    std::move(buses_set)
                    );
    }

    return true;
}

bool MapRenderer::Serialize(proto::MapRenderer &proto_renderer) const {

    auto fill_proto_color = [](proto::svg::Color* proto_color,
            const svg::Color& color_variant)
    {
        if (std::holds_alternative<std::string>(color_variant)) {
            proto_color->set_str_value(std::get<std::string>(color_variant));
        }
        else if (std::holds_alternative<svg::Rgb>(color_variant)) {
            const auto& color {std::get<svg::Rgb>(color_variant)};
            auto proto_rgb_color {proto_color->mutable_rgb_value()};
            proto_rgb_color->set_red(color.red);
            proto_rgb_color->set_green(color.green);
            proto_rgb_color->set_blue(color.blue);
        }
        else if (std::holds_alternative<svg::Rgba>(color_variant)) {
            const auto& color {std::get<svg::Rgba>(color_variant)};
            auto proto_rgba_color {proto_color->mutable_rgba_value()};
            proto_rgba_color->set_red(color.red);
            proto_rgba_color->set_green(color.green);
            proto_rgba_color->set_blue(color.blue);
            proto_rgba_color->set_opacity(color.opacity);
        }
    };

    auto proto_settings = proto_renderer.mutable_settings();
    proto_settings->set_width(m_settings.width);
    proto_settings->set_height(m_settings.height);
    proto_settings->set_padding(m_settings.padding);
    proto_settings->set_line_width(m_settings.line_width);
    proto_settings->set_stop_radius(m_settings.stop_radius);
    proto_settings->set_bus_label_font_size(m_settings.bus_label_font_size);
    proto_settings->mutable_bus_label_offset()->set_x(m_settings.bus_label_offset.x);
    proto_settings->mutable_bus_label_offset()->set_y(m_settings.bus_label_offset.y);
    proto_settings->set_stop_label_font_size(m_settings.stop_label_font_size);
    proto_settings->mutable_stop_label_offset()->set_x(m_settings.stop_label_offset.x);
    proto_settings->mutable_stop_label_offset()->set_y(m_settings.stop_label_offset.y);
    fill_proto_color(proto_settings->mutable_underlayer_color(),
                     m_settings.underlayer_color);
    proto_settings->set_underlayer_width(m_settings.underlayer_width);

    for (const auto& color : m_settings.color_palette) {
        auto proto_color = proto_settings->add_color_palette();
        fill_proto_color(proto_color, color);
    }

    return true;
}

bool MapRenderer::Deserialize(const proto::MapRenderer& proto_renderer)
{

    auto load_color = [](const proto::svg::Color& proto_color)
    {
        if(proto_color.variant_case() == proto::svg::Color::kStrValue) {
            return svg::Color {proto_color.str_value()};
        }
        else if(proto_color.variant_case() == proto::svg::Color::kRgbValue) {
            svg::Rgb rgb;
            rgb.red = static_cast<uint8_t>(proto_color.rgb_value().red());
            rgb.green = static_cast<uint8_t>(proto_color.rgb_value().green());
            rgb.blue = static_cast<uint8_t>(proto_color.rgb_value().blue());
            return svg::Color {std::move(rgb)};
        }
        else if(proto_color.variant_case() == proto::svg::Color::kRgbaValue) {
            svg::Rgba rgba;
            rgba.red = static_cast<uint8_t>(proto_color.rgba_value().red());
            rgba.green = static_cast<uint8_t>(proto_color.rgba_value().green());
            rgba.blue = static_cast<uint8_t>(proto_color.rgba_value().blue());
            rgba.opacity = proto_color.rgba_value().opacity();
            return svg::Color {std::move(rgba)};
        }
        return svg::Color {};
    };

    const auto& proto_settings {proto_renderer.settings()};
    RenderSettings settings;
    settings.width = proto_settings.width();
    settings.height = proto_settings.height();
    settings.padding = proto_settings.padding();
    settings.line_width = proto_settings.line_width();
    settings.stop_radius = proto_settings.stop_radius();
    settings.bus_label_font_size = proto_settings.bus_label_font_size();
    settings.bus_label_offset.x = proto_settings.bus_label_offset().x();
    settings.bus_label_offset.y = proto_settings.bus_label_offset().y();
    settings.stop_label_font_size = proto_settings.stop_label_font_size();
    settings.stop_label_offset.x = proto_settings.stop_label_offset().x();
    settings.stop_label_offset.y = proto_settings.stop_label_offset().y();
    settings.underlayer_color = load_color(proto_settings.underlayer_color());
    settings.underlayer_width = proto_settings.underlayer_width();

    for (const auto& proto_color : proto_settings.color_palette()) {
        settings.color_palette.push_back(load_color(proto_color));
    }

    SetSettings(settings);
    return true;
}

namespace transport {
bool Router::Serialize(proto::transport::Router& proto_router) const {
    auto proto_settings = proto_router.mutable_settings();
    proto_settings->set_bus_wait_time(m_settings.bus_wait_time);
    proto_settings->set_bus_velocity(m_settings.bus_velocity);

    auto proto_graph = proto_router.mutable_graph();
    m_graph->Serialise(*proto_graph);

    auto proto_vertex_to_name = proto_router.mutable_vertex_to_name();
    for(const auto& [id, name] : m_vertex_to_name) {
        (*proto_vertex_to_name)[id] = std::string(name);
    }

    auto proto_name_to_vertex_wait = proto_router.mutable_name_to_vertex_wait();
    for(const auto& [name, id] : m_name_to_vertex_wait) {
        (*proto_name_to_vertex_wait)[std::string(name)] = id;
    }

    auto proto_name_to_vertex_go = proto_router.mutable_name_to_vertex_go();
    for(const auto& [name, id] : m_name_to_vertex_go) {
        (*proto_name_to_vertex_go)[std::string(name)] = id;
    }

    auto proto_edge_to_data = proto_router.mutable_edge_to_data();
    for(const auto& [edge, data] : m_edge_to_data) {
        auto& proto_data = (*proto_edge_to_data)[edge] = {};
        proto_data.set_bus(std::string(data.bus));
        proto_data.set_span_count(data.span_count);
        proto_data.set_is_wait(data.is_wait);
    }

    return true;
}

bool Router::Deserialize(const proto::transport::Router &proto_router) {

    m_settings.bus_wait_time = proto_router.settings().bus_wait_time();
    m_settings.bus_velocity = proto_router.settings().bus_velocity();

    const auto nodes_count {m_transport_catalogue.GetStops().size()};
    m_graph = std::make_unique<graph::DirectedWeightedGraph<double>>(2 * nodes_count);
    m_graph->Deserialise(proto_router.graph());
    m_router = std::make_unique<graph::Router<double>>(*m_graph);

    for(const auto& [vertex, name] : proto_router.vertex_to_name()) {
        m_vertex_to_name[vertex] = m_transport_catalogue.GetStop(name)->name;
    }

    for(const auto& [name, vertex] : proto_router.name_to_vertex_wait()) {
        m_name_to_vertex_wait[m_transport_catalogue.GetStop(name)->name] = vertex;
    }

    for(const auto& [name, vertex] : proto_router.name_to_vertex_go()) {
        m_name_to_vertex_go[m_transport_catalogue.GetStop(name)->name] = vertex;
    }

    for(const auto& [edge, data] : proto_router.edge_to_data()) {
        m_edge_to_data[edge] =
                EdgeData {
                    m_transport_catalogue.GetBus(data.bus())->name,
                    data.span_count(),
                    data.is_wait()
                };
    }

    return true;
}

} //namespace transport
