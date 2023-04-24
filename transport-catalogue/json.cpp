#include "json.h"

#include <algorithm>

namespace json {

namespace {
using namespace std::string_view_literals;
using namespace std::string_literals;

Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;
    char c = 0;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (c != ']') throw ParsingError("Failed to read number from stream");
    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream");
        }
    };

    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected");
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadNull(std::istream& input) {
    std::string null(4, '\0');
    input.read(&null[0], 4);
    if (null != "null"sv) {
        throw ParsingError("Failed to read null");
    }

    return {};
}

Node LoadDict(std::istream& input) {
    Dict result;
    char c = 0;
    std::string key;
    while(input >> c) {
        if (c == '}') break;
        if (c == ',') {
            key.clear();
        } else if (c == '"') {
            key = LoadString(input).AsString();
        } else if (c == ':') {
            result.insert({move(key), LoadNode(input)});
        }
    }
    if (c != '}') throw ParsingError("Failed to read number from stream"s);
    return Node(move(result));
}

Node LoadBool(std::istream& input) {

    std::string line(5, '\0');

    bool is_true {input.peek() == 't'};
    if (is_true){
        input.read(&line[0], 4);
        line.resize(4);
        if (line != "true"sv) throw ParsingError("Failed to read Bool"s);
    } else {
        input.read(&line[0], 5);
        if (line != "false"sv) throw ParsingError("Failed to read Bool"s);
    }
    return Node(is_true);
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == '-' || (c >= '0' && c <= '9')) {
        input.putback(c);
        return LoadNumber(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else  if (c == ' ' || c == '\n' || c == '\t') {
        input >> c;
        return LoadNode(input);
    } else {
        input.putback(c);
        return LoadNull(input);
    }
}

} //namespace

Node::Node(std::nullptr_t)
    : value_ {nullptr}
{}

Node::Node(Array array)
    : value_(move(array))
{}

Node::Node(Dict map)
    : value_(move(map))
{}

Node::Node(bool value)
    : value_(value)
{}

Node::Node(int value)
    : value_(value)
{}

Node::Node(double value)
    : value_(value)
{}

Node::Node(std::string value)
    : value_(move(value))
{}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}
bool Node::IsDouble() const {
    return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
}
bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}
bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}
bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}
bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}
bool Node::IsMap() const {
    return std::holds_alternative<Map>(value_);
}

int Node::AsInt() const {
    if (!IsInt())  throw std::logic_error("Not an Int");
    return std::get<int>(value_);
}
const std::string& Node::AsString() const {
    if (!IsString())  throw std::logic_error("Not a String");
    return std::get<std::string>(value_);
}
const Array& Node::AsArray() const {
    if (!IsArray())  throw std::logic_error("Not an Array");
    return std::get<Array>(value_);
}
const Dict& Node::AsMap() const {
    if (!IsMap()) throw std::logic_error("Not a Map");
    return std::get<Dict>(value_);
}
bool Node::AsBool() const {
    if (!IsBool())  throw std::logic_error("Not a Bool");
    return std::get<bool>(value_);
}
double Node::AsDouble() const {
    if (IsInt()) return static_cast<double>(std::get<int>(value_));
    if (!IsDouble()) throw std::logic_error("Not a Double");
    return std::get<double>(value_);
}

bool Node::operator==(const Node& rhs) const {
    return value_ == rhs.value_;
}

bool Node::operator!=(const Node& rhs) const {
    return value_ != rhs.value_;
}

Document::Document(Node root)
    : root_ {std::move(root)}
{}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

bool Document::operator==(const Document& rhs) const {
    return root_ == rhs.root_;
}
bool Document::operator!=(const Document& rhs) const{
    return root_ != rhs.root_;
}

struct ValuePrinter {
    std::ostream& out;
    size_t tab_num {0};
    std::string tab {"    "};


    void indent() const {
        for (auto i = 0u; i < tab_num; ++i){
            out << tab;
        }
    }

    template <typename Value>
    void operator()(Value v) const {
        out << v;
    }

    void operator()(std::nullptr_t) const {
        out << "null"sv;
    }

    void operator()(std::vector<Node> v) const {
        out << "[\n" << tab;
        for (size_t i = 0; i < v.size() ; ++i) {
            out << tab;
            PrintNode(v[i], out);
            if (i != v.size() - 1) {
                out << ",\n";
            }
        }
        out << "\n]";
    }

    void operator()(std::map<std::string, Node> m) const {
        out << "{\n";
        for (auto it = m.begin(); it != m.end(); it++) {
            out << tab << '"' << it->first << '"' << ": ";
            PrintNode(it->second, out);
            if (it != std::prev(m.end())) {
                out << ",\n";
            }
        }
        out << "\n}";
    }

    void operator()(bool b) const {
        out << (b ? "true"sv : "false"sv);
    }

    void operator()(const std::string& s) const {
        out << '"';
        for (char c : s) {
            switch (c) {
            case '"':
                out << "\\\"";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\n':
                out << "\\n";
                break;
            default:
                out << c;
            }
        }
        out << '"';
    }

};

void PrintNode(const Node& node, std::ostream& out, size_t indent) {
    std::visit(ValuePrinter{out, indent}, node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    const Node& root = doc.GetRoot();
    PrintNode(root, output);
}

}  // namespace json
