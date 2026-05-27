#ifndef INTELLISEARCH_SEARCH_ENGINE_H
#define INTELLISEARCH_SEARCH_ENGINE_H

#include "analytics.h"
#include "config.h"
#include "query_expander.h"
#include "trie.h"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <unordered_set>
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
    double bm25_score;
    double expansion_score;
    double fuzzy_score;
    double phrase_boost;
    std::string snippet;
    std::vector<std::string> matched_terms;
    std::vector<std::string> fuzzy_matches;
    std::vector<std::string> expansions;
};

class SearchEngine
{
public:
    explicit SearchEngine(EngineConfig config = EngineConfig());

    bool load_documents(const std::string &path, std::string &error);
    bool load_expansion_dictionary(const std::string &path, std::string &error);
    std::vector<SearchResult> search(const std::vector<std::string> &query_terms,
                                     std::size_t top_k);
    std::vector<std::string> autocomplete(const std::string &prefix, std::size_t limit);

    std::size_t document_frequency(const std::string &term) const;
    std::size_t term_frequency(std::size_t doc_id, const std::string &term) const;
    void print_document_frequencies(std::ostream &out) const;
    void print_analytics(std::ostream &out) const;

    std::size_t document_count() const;
    std::size_t total_terms() const;
    double average_document_length() const;
    const EngineConfig &config() const;

private:
    enum class TermSource
    {
        Exact,
        Expansion,
        Fuzzy
    };

    struct WeightedTerm
    {
        std::string term;
        std::string origin;
        TermSource source;
        double weight;
        std::size_t edit_distance;
    };

    std::vector<std::string> normalize_query_terms(const std::vector<std::string> &query_terms) const;
    std::vector<WeightedTerm> build_weighted_terms(const std::vector<std::string> &terms) const;
    double phrase_boost_for_document(const Document &document,
                                     const std::vector<std::vector<std::string> > &phrases) const;
    std::vector<std::vector<std::string> > phrase_candidates(const std::vector<std::string> &terms) const;
    bool contains_phrase(const std::vector<std::string> &tokens,
                         const std::vector<std::string> &phrase) const;
    std::string make_snippet(const Document &document,
                             const std::unordered_set<std::string> &highlight_terms) const;
    std::string join_terms(const std::vector<std::string> &terms) const;
    void add_unique(std::vector<std::string> &values, const std::string &value) const;

    EngineConfig config_;
    Trie index_;
    parsing::QueryExpander query_expander_;
    SearchAnalytics analytics_;
    std::vector<Document> documents_;
    std::vector<std::string> vocabulary_;
    std::unordered_set<std::string> vocabulary_lookup_;
    std::size_t total_terms_;
};

} // namespace intellisearch

#endif
