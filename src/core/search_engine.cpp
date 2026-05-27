#include "search_engine.h"

#include "bm25.h"
#include "tokenizer.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_set>

namespace intellisearch
{

SearchEngine::SearchEngine(EngineConfig config)
    : config_(config), index_(), documents_(), total_terms_(0)
{
}

bool SearchEngine::load_documents(const std::string &path, std::string &error)
{
    std::ifstream input(path.c_str());
    if (!input)
    {
        error = "Unable to open dataset: " + path;
        return false;
    }

    index_.clear();
    documents_.clear();
    total_terms_ = 0;

    std::string line;
    std::size_t expected_id = 0;
    std::size_t line_number = 0;

    while (std::getline(input, line))
    {
        ++line_number;
        if (parsing::trim(line).empty())
        {
            continue;
        }

        std::istringstream stream(line);
        std::string id_text;
        stream >> id_text;

        std::size_t doc_id = 0;
        if (!parsing::parse_unsigned(id_text, doc_id))
        {
            std::ostringstream message;
            message << "Line " << line_number << " has a non-numeric document id.";
            error = message.str();
            return false;
        }

        if (doc_id != expected_id)
        {
            std::ostringstream message;
            message << "Line " << line_number << " has document id " << doc_id
                    << ", expected " << expected_id << ".";
            error = message.str();
            return false;
        }

        std::string content;
        std::getline(stream, content);
        content = parsing::trim(content);

        Document document;
        document.id = doc_id;
        document.text = content;
        document.tokens = parsing::tokenize(content);

        std::istringstream raw_terms(content);
        std::string raw_term;
        while (raw_terms >> raw_term)
        {
            if (!parsing::normalize_token(raw_term).empty())
            {
                document.raw_terms.push_back(raw_term);
            }
        }

        for (const std::string &token : document.tokens)
        {
            index_.insert_word(token, doc_id);
        }

        total_terms_ += document.tokens.size();
        documents_.push_back(document);
        ++expected_id;
    }

    if (documents_.empty())
    {
        error = "Dataset contains no documents.";
        return false;
    }

    return true;
}

std::vector<SearchResult> SearchEngine::search(const std::vector<std::string> &query_terms,
                                               std::size_t top_k) const
{
    std::vector<std::string> terms = normalize_query_terms(query_terms);
    std::vector<const PostingList *> posting_lists;
    std::set<std::size_t> candidate_ids;

    for (const std::string &term : terms)
    {
        const PostingList *posting_list = index_.search_word(term);
        posting_lists.push_back(posting_list);

        if (posting_list == nullptr)
        {
            continue;
        }

        for (std::size_t doc_id : posting_list->document_ids())
        {
            candidate_ids.insert(doc_id);
        }
    }

    std::vector<SearchResult> results;
    const double avgdl = average_document_length();

    for (std::size_t doc_id : candidate_ids)
    {
        double score = 0.0;
        for (std::size_t i = 0; i < terms.size(); ++i)
        {
            const PostingList *posting_list = posting_lists[i];
            if (posting_list == nullptr)
            {
                continue;
            }

            score += ranking::bm25_score(
                posting_list->term_frequency(doc_id),
                posting_list->document_frequency(),
                avgdl,
                documents_[doc_id].tokens.size(),
                documents_.size(),
                config_.bm25_k1,
                config_.bm25_b);
        }

        results.push_back({0, doc_id, score, make_snippet(documents_[doc_id], terms)});
    }

    std::sort(results.begin(), results.end(),
              [](const SearchResult &left, const SearchResult &right) {
                  if (left.score == right.score)
                  {
                      return left.doc_id < right.doc_id;
                  }
                  return left.score > right.score;
              });

    if (top_k == 0)
    {
        top_k = config_.top_k;
    }

    if (results.size() > top_k)
    {
        results.resize(top_k);
    }

    for (std::size_t i = 0; i < results.size(); ++i)
    {
        results[i].rank = i + 1;
    }

    return results;
}

std::vector<std::string> SearchEngine::autocomplete(const std::string &prefix, std::size_t limit) const
{
    const std::string normalized = parsing::normalize_token(prefix);
    if (normalized.empty())
    {
        return std::vector<std::string>();
    }

    return index_.autocomplete(normalized, limit == 0 ? config_.autocomplete_limit : limit);
}

std::size_t SearchEngine::document_frequency(const std::string &term) const
{
    const std::string normalized = parsing::normalize_token(term);
    const PostingList *posting_list = index_.search_word(normalized);
    return posting_list == nullptr ? 0 : posting_list->document_frequency();
}

std::size_t SearchEngine::term_frequency(std::size_t doc_id, const std::string &term) const
{
    if (doc_id >= documents_.size())
    {
        return 0;
    }

    const std::string normalized = parsing::normalize_token(term);
    const PostingList *posting_list = index_.search_word(normalized);
    return posting_list == nullptr ? 0 : posting_list->term_frequency(doc_id);
}

void SearchEngine::print_document_frequencies(std::ostream &out) const
{
    index_.print(false, out);
}

std::size_t SearchEngine::document_count() const
{
    return documents_.size();
}

std::size_t SearchEngine::total_terms() const
{
    return total_terms_;
}

double SearchEngine::average_document_length() const
{
    if (documents_.empty())
    {
        return 0.0;
    }

    return static_cast<double>(total_terms_) / static_cast<double>(documents_.size());
}

const EngineConfig &SearchEngine::config() const
{
    return config_;
}

std::vector<std::string> SearchEngine::normalize_query_terms(const std::vector<std::string> &query_terms) const
{
    std::vector<std::string> terms;
    std::unordered_set<std::string> seen;

    for (const std::string &term : query_terms)
    {
        const std::string normalized = parsing::normalize_token(term);
        if (normalized.empty() || seen.count(normalized) != 0)
        {
            continue;
        }

        terms.push_back(normalized);
        seen.insert(normalized);

        if (terms.size() == config_.max_query_terms)
        {
            break;
        }
    }

    return terms;
}

std::string SearchEngine::make_snippet(const Document &document, const std::vector<std::string> &query_terms) const
{
    if (document.tokens.empty() || document.raw_terms.empty())
    {
        return document.text;
    }

    std::unordered_set<std::string> query_set(query_terms.begin(), query_terms.end());
    std::size_t first_match = document.raw_terms.size();

    for (std::size_t i = 0; i < document.raw_terms.size(); ++i)
    {
        if (query_set.count(parsing::normalize_token(document.raw_terms[i])) != 0)
        {
            first_match = i;
            break;
        }
    }

    if (first_match == document.raw_terms.size())
    {
        return document.text;
    }

    const std::size_t start = first_match > config_.snippet_radius ? first_match - config_.snippet_radius : 0;
    const std::size_t end = std::min(document.raw_terms.size(), first_match + config_.snippet_radius + 1);

    std::ostringstream snippet;
    if (start > 0)
    {
        snippet << "... ";
    }

    for (std::size_t i = start; i < end; ++i)
    {
        if (i > start)
        {
            snippet << ' ';
        }

        if (query_set.count(parsing::normalize_token(document.raw_terms[i])) != 0)
        {
            snippet << '[' << document.raw_terms[i] << ']';
        }
        else
        {
            snippet << document.raw_terms[i];
        }
    }

    if (end < document.raw_terms.size())
    {
        snippet << " ...";
    }

    return snippet.str();
}

} // namespace intellisearch
