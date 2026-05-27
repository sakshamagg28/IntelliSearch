#include "fuzzy.h"

#include <algorithm>
#include <limits>
#include <vector>

namespace intellisearch
{
namespace utils
{

std::size_t edit_distance(const std::string &left, const std::string &right,
                          std::size_t max_distance)
{
    const std::size_t impossible = max_distance + 1;
    const std::size_t left_size = left.size();
    const std::size_t right_size = right.size();

    if (left_size > right_size + max_distance || right_size > left_size + max_distance)
    {
        return impossible;
    }

    std::vector<std::size_t> previous(right_size + 1);
    std::vector<std::size_t> current(right_size + 1);

    for (std::size_t j = 0; j <= right_size; ++j)
    {
        previous[j] = j;
    }

    for (std::size_t i = 1; i <= left_size; ++i)
    {
        current[0] = i;
        std::size_t row_min = current[0];

        for (std::size_t j = 1; j <= right_size; ++j)
        {
            const std::size_t substitution_cost = left[i - 1] == right[j - 1] ? 0 : 1;
            current[j] = std::min(
                std::min(previous[j] + 1, current[j - 1] + 1),
                previous[j - 1] + substitution_cost);
            row_min = std::min(row_min, current[j]);
        }

        if (row_min > max_distance)
        {
            return impossible;
        }

        previous.swap(current);
    }

    return previous[right_size] <= max_distance ? previous[right_size] : impossible;
}

std::vector<FuzzyMatch> find_fuzzy_matches(const std::string &term,
                                           const std::vector<std::string> &vocabulary,
                                           std::size_t max_distance,
                                           std::size_t limit)
{
    std::vector<FuzzyMatch> matches;

    if (term.empty() || max_distance == 0 || limit == 0)
    {
        return matches;
    }

    for (const std::string &candidate : vocabulary)
    {
        if (candidate == term)
        {
            continue;
        }

        const std::size_t distance = edit_distance(term, candidate, max_distance);
        if (distance <= max_distance)
        {
            matches.push_back({candidate, distance});
        }
    }

    std::sort(matches.begin(), matches.end(),
              [](const FuzzyMatch &left, const FuzzyMatch &right) {
                  if (left.distance == right.distance)
                  {
                      return left.term < right.term;
                  }
                  return left.distance < right.distance;
              });

    if (matches.size() > limit)
    {
        matches.resize(limit);
    }

    return matches;
}

} // namespace utils
} // namespace intellisearch
