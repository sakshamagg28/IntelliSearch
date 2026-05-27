#ifndef INTELLISEARCH_FUZZY_H
#define INTELLISEARCH_FUZZY_H

#include <cstddef>
#include <string>
#include <vector>

namespace intellisearch
{
namespace utils
{

struct FuzzyMatch
{
    std::string term;
    std::size_t distance;
};

std::size_t edit_distance(const std::string &left, const std::string &right,
                          std::size_t max_distance);
std::vector<FuzzyMatch> find_fuzzy_matches(const std::string &term,
                                           const std::vector<std::string> &vocabulary,
                                           std::size_t max_distance,
                                           std::size_t limit);

} // namespace utils
} // namespace intellisearch

#endif
