#ifndef INTELLISEARCH_ANALYTICS_H
#define INTELLISEARCH_ANALYTICS_H

#include <cstddef>
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

namespace intellisearch
{

class SearchAnalytics
{
public:
    void record_query(const std::string &query);
    void record_autocomplete(const std::string &prefix);
    void print(std::ostream &out, std::size_t limit) const;

private:
    std::vector<std::pair<std::string, std::size_t> > top_entries(
        const std::unordered_map<std::string, std::size_t> &counts,
        std::size_t limit) const;

    std::unordered_map<std::string, std::size_t> query_counts_;
    std::unordered_map<std::string, std::size_t> autocomplete_counts_;
};

} // namespace intellisearch

#endif
