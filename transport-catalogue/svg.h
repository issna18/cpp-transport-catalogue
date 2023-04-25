#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

struct Rgb {
    Rgb()  = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue)
        : red {red},
          green {green},
          blue {blue}
    {}

    uint8_t red {0};
    uint8_t green {0};
    uint8_t blue {0};
};

struct Rgba {
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
        : red {red},
          green {green},
          blue {blue},
          opacity {opacity}
    {}

    uint8_t red {0};
    uint8_t green {0};
    uint8_t blue {0};
    double opacity {1.0};
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor{"none"};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

struct Point {
    Point() = default;
    Point(double ax, double ay)
        : x {ax},
          y {ay}
    {}

    double x {0};
    double y {0};
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& aout)
        : out {aout}
    {}

    RenderContext(std::ostream& aout, int aindent_step, int aindent = 0)
        : out {aout},
          indent_step {aindent_step},
          indent {aindent}
    {}

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step {0};
    int indent {0};
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);

struct ColorPrinter {

    std::ostream& out;

    void operator()(std::monostate) const {
        out << std::string("none");
    }
    void operator()(const std::string& str) const {
        out << str;
    }
    void operator()(const Rgb& rgb) const {
        out << "rgb(" << static_cast<int>(rgb.red) << "," << static_cast<int>(rgb.green) << "," << static_cast<int>(rgb.blue) << ")";
    }
    void operator()(const Rgba& rgba) const {
        out << "rgba(" << static_cast<int>(rgba.red) << "," << static_cast<int>(rgba.green) << "," << static_cast<int>(rgba.blue) << "," << rgba.opacity << ")";
    }
};

std::ostream& operator<<(std::ostream& out, const Color& color);

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(int width) {
        width_ = width;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin stroke_line_join) {
        stroke_line_join_ = std::move(stroke_line_join);
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap stroke_line_cap) {
        stroke_line_cap_ = std::move(stroke_line_cap);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv;
            std::visit(ColorPrinter{ std::cout }, *fill_color_);
            out << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv;
            std::visit(ColorPrinter{ std::cout }, *stroke_color_);
            out << "\""sv;
        }
        if (width_) {
            out << " stroke-width=\""sv << *width_ << "\""sv;
        }
        if (stroke_line_cap_) {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
        }
        if (stroke_line_join_) {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<int> width_;
    std::optional<StrokeLineJoin> stroke_line_join_;
    std::optional<StrokeLineCap> stroke_line_cap_;
};

//-----------Circle-----------------------------

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ {1.0};
};

//-----------Polyline-----------------------------

class Polyline final: public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    std::vector<Point> points_;
    void RenderObject(const RenderContext& context) const override;
};

//-----------Text-----------------------------

class Text : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    double x_ {0.0};
    double y_ {0.0};
    double dx_ {0.0};
    double dy_ {0.0};
    uint32_t size_ {1};
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
    void RenderObject(const RenderContext& context) const override;
};

class ObjectContainer {
public:

    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    virtual ~ObjectContainer() = default;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    virtual ~Document() = default;

private:
    std::deque<std::unique_ptr<Object>> objects_;

};

}  // namespace svg
