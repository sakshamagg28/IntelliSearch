#ifndef INTELLISEARCH_QUERY_EXPANDER_H
#define INTELLISEARCH_QUERY_EXPANDER_H

#include <string>
#include <unordered_map>
#include <vector>

namespace intellisearch
{
namespace parsing
{

class QueryExpander
{
public:
    QueryExpander();

    bool load_from_file(const std::string &path, std::string &error);
    void set_expansion(const std::string &term, const std::vector<std::string> &expansion_terms);
    std::vector<std::string> expand(const std::string &term) const;

private:
    std::unordered_map<std::string, std::vector<std::string> > expansions_;
};

} // namespace parsing
} // namespace intellisearch

#endif
