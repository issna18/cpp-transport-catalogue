#include "svg.h"
#include <sstream>

namespace svg {

using namespace std::literals;

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red {r},
          green {g},
          blue {b}
    {}

Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
        : red {r},
          green {g},
          blue {b},
          opacity {op}
    {}

void ColorPrinter::operator()(std::monostate) const {
    out << "none"s;
}

void ColorPrinter::operator()(const std::string& str) const {
    out << str;
}

void ColorPrinter::operator()(const Rgb& rgb) const {
    int red = rgb.red;
    int green = rgb.green;
    int blue = rgb.blue;
    out << "rgb("s << red
        << ","s << green
        << ","s << blue << ")"s;
}

void ColorPrinter::operator()(const Rgba& rgba) const {
    int red = rgba.red;
    int green = rgba.green;
    int blue = rgba.blue;
    double opacity = rgba.opacity;
    out << "rgba("s << red
        << ","s << green
        << ","s << blue
        << ","s << opacity << ")"s;
}

Point::Point(double ax, double ay)
        : x {ax},
          y {ay}
    {}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    switch (line_cap) {
    case StrokeLineCap::BUTT: out << "butt"sv; break;
    case StrokeLineCap::ROUND: out << "round"sv; break;
    case StrokeLineCap::SQUARE: out << "square"sv; break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    switch (line_join) {
    case StrokeLineJoin::ARCS: out << "arcs"sv; break;
    case StrokeLineJoin::BEVEL: out << "bevel"sv; break;
    case StrokeLineJoin::MITER: out << "miter"sv; break;
    case StrokeLineJoin::MITER_CLIP: out << "miter-clip"sv; break;
    case StrokeLineJoin::ROUND: out << "round"sv; break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& output, const Color& color) {
        std::ostringstream out;
        std::visit(ColorPrinter{out}, color);
        output << out.str();
        return output;
    }

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

    // Добавляет очередную вершину к ломаной линии
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for (auto it = points_.begin(); it != points_.end(); ++it) {
        out << it->x << ',' << it->y;
        if (it != points_.end() - 1) {
            out << ' ';
        }
    }
    out << '"';
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text ------------------

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    x_ = pos.x;
    y_ = pos.y;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    dx_ = offset.x;
    dy_ = offset.y;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
     auto& out = context.out;

    out << "<text";
    RenderAttrs(out);
    out << " x=\"" << x_
        << "\" y=\"" << y_
        << "\" dx=\"" << dx_
        << "\" dy=\"" << dy_
        << "\" font-size=\"" << size_;
    if(!font_family_.empty()) out << "\" font-family=\"" << font_family_;
    if(!font_weight_.empty()) out << "\" font-weight=\"" << font_weight_;

    out << "\">" << data_ << "</text>";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx(out, 2);
    for (const auto& obj : objects_) {
        //obj->Render(out);
        obj->Render(ctx.Indented());
    }
    out << "</svg>\n"sv;
}

}  // namespace svg
