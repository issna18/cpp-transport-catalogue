#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Array = std::vector<Node>;
using Dict = std::map<std::string, Node>;
using Map = Dict;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    Node() = default;
    Node(std::nullptr_t);
    Node(Array array);
    Node(Dict map);
    Node(bool value);
    Node(int value);
    Node(double value);
    Node(std::string value);

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

    const Value& GetValue() const { return value_; }

private:
    Value value_;
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

void Print(const Document& doc, std::ostream& output);

}  // namespace json
