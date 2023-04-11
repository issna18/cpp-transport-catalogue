#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace Reader {

class Stat
{
public:
    using Query = std::pair<bool, std::string>;
    Stat(std::istream& input);
    const std::vector<Query>& GetQueries();

private:
    void ProcessLine(std::string_view line);
    Query ParseQuery(std::string_view line);
    std::vector<Query> queries;
};
}
