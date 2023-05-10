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

/*
В данном файле находится класс Builder и вспомогательные классы:
    - ArrayContext
    - DictContext
    - KeyContext
    - BaseContext

Класс Builder занимается построением объектной модели JSON документа
и содержит корневой узел в приватном поле `root_`.
Публичные методы класса Builder:
    - Key(std::string key) - создает узел ключа в словаре.
                             Ключом может быть только строка.
    - Value(const Node::Value& value) - создает узел значения в словаре, в массиве,
                                        либо является корнем документа
    - StartDict() - создает узел словаря
    - StartArray() - создает узел массива
    - EndDict() - вспомогательный метод для закрытия сформированного словаря
    - EndArray() - вспомогательный метод для закрытия сформированного массива
    - Build() - возвращает корневой узел объектной модели JSON документа.

При использовании только класса Builder есть большая вероятность
допустить ошибку, например:
`json::Builder{}.StartDict().Key("1"s).Key(""s);` - нет value после key.

Так происходило раньше,
когда методы объекта Builder возрашали ссылку на него самого. У пользователя
не было ограничения на вызов последующего метода.

В процессе выполнения программы при неправильных вызовах бросаются исключения.
Но лучше найти ошибку как можно раньше.

Чтобы на этапе сборки компилятор сам определял ошибочные ситуации, были сделаны
вспомогательные классы: ArrayContext, DictContext, KeyContext, BaseContext.
Эти классы (далее Context классы) являются обертками над классом Builder,
они не наследуются от него, а содержат ссылку на него.

Каждый из Context классов содержит не все методы Builder,
а только те которые разрешены в текущем состоянии модели JSON документа.
Например, вызов StartArray() вернёт ArrayContext, а класс ArrayContext позволит
вызвать только:
    - Value(const Node::Value& value)
    - StartDict()
    - StartArray()
    - EndArray(),

а вызов StartDict() вернёт DictContext, который позволит:
    - Key(std::string key)
    - EndDict().


Все названия методов Context классов соответствуют названиям в классе Builder,
и, как правило, имеют вид:
```
DictContext ArrayContext::StartDict() {
    return m_builder.StartDict();
}
```
Т.е. вызывают и возвращают результат того же метода Builder.

**Чтобы этот механизм заработал нужно сделать три вещи:**
1. Переделать методы Builder-а так, чтобы они возвращали правильный Context класс

2. Метод Builder-а будет передавать ссылку на себя в конструктор Context класса,
и возвращать сконструированный объект соответствующего Context класса.
Чтобы не дублировать в каждом Context классе ссылку на Builder
и её инициализацию, этот код вынесен в базовый класс BaseContext.
При декларации классов ArrayContext, DictContext, KeyContext -
они наследуются от BaseContext, а их конструктор делегирует инициализацию
конструктору класса BaseContext.
Все конструкторы Context классов намеренно сделаны не explicit,
чтобы в методах Builder-а компилятор сам конвертировал
`return *this` в нужный Context класс, например
вместо`return ArrayContext(*this)` => `return *this`.
Возможно это нужно делать явно и все конструкторы сделать explicit.

3. Методы  Context классов также возвращают соответствующие Context классы,
чтобы компилятор сам проверял, что вызывает пользователь.

*/
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
