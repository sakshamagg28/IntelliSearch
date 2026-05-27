#ifndef INTELLISEARCH_SEARCH_ENGINE_H
#define INTELLISEARCH_SEARCH_ENGINE_H

#include "config.h"
#include "trie.h"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace intellisearch
{

struct Document
{
    std::size_t id;
    std::string text;
    std::vector<std::string> tokens;
    std::vector<std::string> raw_terms;
};

struct SearchResult
{
    std::size_t rank;
    std::size_t doc_id;
    double score;
    std::string snippet;
};

class SearchEngine
{
public:
    explicit SearchEngine(EngineConfig config = EngineConfig());

    bool load_documents(const std::string &path, std::string &error);
    std::vector<SearchResult> search(const std::vector<std::string> &query_terms,
                                     std::size_t top_k) const;
    std::vector<std::string> autocomplete(const std::string &prefix, std::size_t limit) const;

    std::size_t document_frequency(const std::string &term) const;
    std::size_t term_frequency(std::size_t doc_id, const std::string &term) const;
    void print_document_frequencies(std::ostream &out) const;

    std::size_t document_count() const;
    std::size_t total_terms() const;
    double average_document_length() const;
    const EngineConfig &config() const;

private:
    std::vector<std::string> normalize_query_terms(const std::vector<std::string> &query_terms) const;
    std::string make_snippet(const Document &document, const std::vector<std::string> &query_terms) const;

    EngineConfig config_;
    Trie index_;
    std::vector<Document> documents_;
    std::size_t total_terms_;
};

} // namespace intellisearch

#endif
