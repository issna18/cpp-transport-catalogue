#pragma once

#include "json.h"

#include <string>
#include <vector>
#include <variant>

namespace json {

class Builder;
class ArrayContext;
class DictContext;
class KeyContext;

class BaseContext {
public:
    BaseContext(Builder& builder)
        : m_builder {builder}
    {}

protected:
    Builder& m_builder;
};

class ArrayContext : private BaseContext {
public:
    ArrayContext(Builder& builder)
        : BaseContext {builder}
    {}

    ArrayContext Value(const Node::Value& value);
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& EndArray();
};

class DictContext : private BaseContext
{
public:
    DictContext(Builder& builder)
        : BaseContext {builder}
    {}

    KeyContext Key(std::string key);
    Builder& EndDict();
};

class KeyContext : private BaseContext {
public:
    KeyContext(Builder& builder)
        : BaseContext {builder}
    {}

    DictContext Value(const Node::Value& value);
    DictContext StartDict();
    ArrayContext StartArray();
};

class Builder {
public:
    KeyContext Key(std::string key);
    Builder& Value(const Node::Value& value);
    DictContext StartDict();
    ArrayContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node& Build();

private:
    Node root_ {nullptr};
    std::vector<Node*> nodes_stack_;
    std::string key_;
    bool key_was_set {false};
};

} //json
