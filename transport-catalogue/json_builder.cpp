#include "json_builder.h"

#include <utility>

namespace json {

Builder& ArrayContext::EndArray() {
    return m_builder.EndArray();
}

ArrayContext ArrayContext::StartArray() {
    return m_builder.StartArray();
}

DictContext ArrayContext::StartDict() {
    return m_builder.StartDict();
}

ArrayContext ArrayContext::Value(const Node::Value& value) {
    return m_builder.Value(value);
}

Builder& DictContext::EndDict() {
    return m_builder.EndDict();
}

KeyContext DictContext::Key(std::string key) {
    return m_builder.Key(std::move(key));
}

ArrayContext KeyContext::StartArray() {
    return m_builder.StartArray();
}

DictContext KeyContext::StartDict() {
     return m_builder.StartDict();
}

DictContext KeyContext::Value(const Node::Value& value) {
    return m_builder.Value(value);
}

KeyContext Builder::Key(std::string key) {
    if (nodes_stack_.empty()) throw std::logic_error("Json invalid");

    if (!nodes_stack_.back()->IsDict() || key_was_set) throw std::logic_error("Key not in Dict");
    key_ = key;
    key_was_set = true;

    return *this;
}

Builder& Builder::Value(const Node::Value& value) {
    if (root_.IsNull()) {
        root_ = value;
        return *this;
    }

    if (nodes_stack_.empty()) throw std::logic_error("Json invalid");

    if (nodes_stack_.back()->IsArray()) {
        Node& back = nodes_stack_.back()->AsArray().emplace_back(Array{});
        back = value;
        return *this;
    }
    if (nodes_stack_.back()->IsDict() && key_was_set) {
        Node node;
        node = value;
        nodes_stack_.back()->AsDict().emplace(std::make_pair(key_, node));
        key_was_set = false;

        return *this;
    }

    throw std::logic_error("invalid value");
    return *this;
}

DictContext Builder::StartDict() {
    if (root_.IsNull()) {
        root_ = Node(Dict{});
        nodes_stack_.push_back(&root_);
        return DictContext(*this);
    }

    if (nodes_stack_.empty()) throw std::logic_error("Json invalid");

    if (nodes_stack_.back()->IsArray()) {
        Node& back = nodes_stack_.back()->AsArray().emplace_back(Dict{});
        nodes_stack_.push_back(&back);

        return DictContext(*this);;
    }

    if (nodes_stack_.back()->IsDict()) {
        if (!key_was_set) throw std::logic_error("No key");
        auto it = nodes_stack_.back()->AsDict().emplace(std::make_pair(key_, Dict{}));
        nodes_stack_.push_back(&(it.first->second));
        key_was_set = false;
        return DictContext(*this);;
    }
    throw std::logic_error("Json invalid");

    return DictContext(*this);
}

ArrayContext Builder::StartArray() {
    if (root_.IsNull()) {
        root_ = Node(Array{});
        nodes_stack_.push_back(&root_);
        return *this;
    }

    if (nodes_stack_.empty()) throw std::logic_error("Json invalid");

    if (nodes_stack_.back()->IsArray()) {
        Node& back = nodes_stack_.back()->AsArray().emplace_back(Array{});
        nodes_stack_.push_back(&back);

        return *this;
    }
    if (nodes_stack_.back()->IsDict()) {
        if (!key_was_set) throw std::logic_error("No key");

        auto it = nodes_stack_.back()->AsDict().emplace(std::make_pair(key_, Array{}));
        nodes_stack_.push_back(&(it.first->second));
        key_was_set = false;
        return *this;
    }

    throw std::logic_error("Json invalid");

    return *this;
}

Builder& Builder::EndDict() {
    if (root_.IsNull()) {
        throw std::logic_error("Not a dict");
    }
    if (nodes_stack_.empty()) throw std::logic_error("Json invalid");

    if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Not a dict");
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (root_.IsNull()) {
        throw std::logic_error("Not an array");
    }
    if (nodes_stack_.empty()) throw std::logic_error("Json invalid");

    if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Not an array");
    }
    nodes_stack_.pop_back();
    return *this;
}

Node& Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Json not finished");
    }
    if (root_.IsNull()) throw std::logic_error("Json invalid");

     if (key_was_set) throw std::logic_error("Json invalid");

    return root_;
}

}
