#ifndef INTELLISEARCH_CONFIG_H
#define INTELLISEARCH_CONFIG_H

#include <cstddef>
#include <string>

namespace intellisearch
{

struct EngineConfig
{
    std::size_t top_k = 10;
    std::size_t max_query_terms = 10;
    std::size_t autocomplete_limit = 8;
    std::size_t snippet_radius = 8;
    std::size_t fuzzy_max_distance = 2;
    std::size_t fuzzy_candidate_limit = 3;
    std::size_t analytics_limit = 10;
    double bm25_k1 = 1.2;
    double bm25_b = 0.75;
    double fuzzy_weight = 0.55;
    double expansion_weight = 0.65;
    double phrase_boost = 1.5;
    std::string expansion_dictionary_path;
};

} // namespace intellisearch

#endif
