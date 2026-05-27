#include "analytics.h"

#include <algorithm>
#include <iostream>

namespace intellisearch
{

void SearchAnalytics::record_query(const std::string &query)
{
    if (!query.empty())
    {
        ++query_counts_[query];
    }
}

void SearchAnalytics::record_autocomplete(const std::string &prefix)
{
    if (!prefix.empty())
    {
        ++autocomplete_counts_[prefix];
    }
}

void SearchAnalytics::print(std::ostream &out, std::size_t limit) const
{
    out << "Search analytics\n";
    out << "----------------\n";

    out << "Top queries:\n";
    const std::vector<std::pair<std::string, std::size_t> > queries = top_entries(query_counts_, limit);
    if (queries.empty())
    {
        out << "  No searches recorded.\n";
    }
    else
    {
        for (const std::pair<std::string, std::size_t> &entry : queries)
        {
            out << "  " << entry.second << "x  " << entry.first << '\n';
        }
    }

    out << "\nAutocomplete prefixes:\n";
    const std::vector<std::pair<std::string, std::size_t> > prefixes =
        top_entries(autocomplete_counts_, limit);
    if (prefixes.empty())
    {
        out << "  No autocomplete usage recorded.\n";
    }
    else
    {
        for (const std::pair<std::string, std::size_t> &entry : prefixes)
        {
            out << "  " << entry.second << "x  " << entry.first << '\n';
        }
    }
}

std::vector<std::pair<std::string, std::size_t> > SearchAnalytics::top_entries(
    const std::unordered_map<std::string, std::size_t> &counts,
    std::size_t limit) const
{
    std::vector<std::pair<std::string, std::size_t> > entries(counts.begin(), counts.end());

    std::sort(entries.begin(), entries.end(),
              [](const std::pair<std::string, std::size_t> &left,
                 const std::pair<std::string, std::size_t> &right) {
                  if (left.second == right.second)
                  {
                      return left.first < right.first;
                  }
                  return left.second > right.second;
              });

    if (entries.size() > limit)
    {
        entries.resize(limit);
    }

    return entries;
}

} // namespace intellisearch
