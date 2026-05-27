#ifndef INTELLISEARCH_CONFIG_H
#define INTELLISEARCH_CONFIG_H

#include <cstddef>

namespace intellisearch
{

struct EngineConfig
{
    std::size_t top_k = 10;
    std::size_t max_query_terms = 10;
    std::size_t autocomplete_limit = 8;
    std::size_t snippet_radius = 8;
    double bm25_k1 = 1.2;
    double bm25_b = 0.75;
};

} // namespace intellisearch

#endif
