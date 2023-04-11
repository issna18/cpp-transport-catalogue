#include "input_reader.h"
#include "stat_reader.h"


#include <istream>
#include <string>
#include <string_view>
#include <vector>

namespace Reader {

Stat::Stat(std::istream& input) {
    std::string str;
    int num_queries;
    input >> num_queries;
    getline(input,str);

    for (int i {0}; i < num_queries; ++i) {
        std::getline(input, str);
        str = Strip(str);
        if (str.empty()) continue;
        ProcessLine(str);
    }
}

const std::vector<Stat::Query>& Stat::GetQueries() {
    return queries;
}

void Stat::ProcessLine(std::string_view line) {
    queries.emplace_back(ParseQuery(line));
};

Stat::Query Stat::ParseQuery(std::string_view line){
    auto query {ParseString(line, ' ')};
    bool is_bus {query.first.front() == 'B'};
    auto name {ParseString(query.second, ':')};
    return std::make_pair(is_bus, name.first);
}

}

