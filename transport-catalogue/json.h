#pragma once

#include <iostream>
#include <set>
#include <string>
#include <variant>
#include <vector>
#include <map>

template <typename F, typename S>
std::ostream& operator<<(std::ostream& out, const std::pair<F, S> p) {
    out << '(' << p.first << ", " << p.second << ')';
    return out;
}

template <typename Container>
std::ostream& Print(std::ostream& out, const Container& container)
{
     bool is_first = true;
    for (const auto& element : container) {
        if (!is_first) {
            out << " ";
        }
        is_first = false;
        out << element;
    }
    return out;
}

template <typename ElementT>
std::ostream& operator<<(std::ostream& out, const std::set<ElementT> container) {
    return Print(out, container);
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& out, const std::map<K, V> container) {
    out << '<';
    return Print(out, container) << '>';
}

namespace json {

class Node;
using Array = std::vector<Node>;
using Dict = std::map<std::string, Node>;
using Map = Dict;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:

    using variant::variant;
    using Value = variant;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Map& AsMap() const;
    bool AsBool() const;
    double AsDouble() const;

    bool operator==(const Node& rhs) const;
    bool operator!=(const Node& rhs) const;

    const Value& GetValue() const { return *this; }

};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs) const;
    bool operator!=(const Document& rhs) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void PrintNode(const Node& node, std::ostream& out, size_t indent = 0u);
void Print(const Document& doc, std::ostream& output);

}  // namespace json
