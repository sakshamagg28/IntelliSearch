#include "search_engine.h"

#include "bm25.h"
#include "fuzzy.h"
#include "tokenizer.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <set>
#include <unordered_set>

namespace intellisearch
{

SearchEngine::SearchEngine(EngineConfig config)
    : config_(config), index_(), query_expander_(), analytics_(), documents_(),
      vocabulary_(), vocabulary_lookup_(), total_terms_(0)
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
    vocabulary_.clear();
    vocabulary_lookup_.clear();
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
            if (vocabulary_lookup_.insert(token).second)
            {
                vocabulary_.push_back(token);
            }
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

bool SearchEngine::load_expansion_dictionary(const std::string &path, std::string &error)
{
    return query_expander_.load_from_file(path, error);
}

std::vector<SearchResult> SearchEngine::search(const std::vector<std::string> &query_terms,
                                               std::size_t top_k)
{
    std::vector<std::string> terms = normalize_query_terms(query_terms);
    analytics_.record_query(join_terms(terms));

    const std::vector<WeightedTerm> weighted_terms = build_weighted_terms(terms);
    const std::vector<std::vector<std::string> > phrases = phrase_candidates(terms);
    std::map<std::string, const PostingList *> posting_lists;
    std::set<std::size_t> candidate_ids;

    for (const WeightedTerm &weighted_term : weighted_terms)
    {
        if (posting_lists.count(weighted_term.term) != 0)
        {
            continue;
        }

        const PostingList *posting_list = index_.search_word(weighted_term.term);
        posting_lists[weighted_term.term] = posting_list;

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
        double bm25_score = 0.0;
        double expansion_score = 0.0;
        double fuzzy_score = 0.0;
        std::unordered_set<std::string> highlight_terms;
        std::vector<std::string> matched_terms;
        std::vector<std::string> expansions;
        std::vector<std::string> fuzzy_matches;

        for (const WeightedTerm &weighted_term : weighted_terms)
        {
            const PostingList *posting_list = posting_lists[weighted_term.term];
            if (posting_list == nullptr)
            {
                continue;
            }

            const double contribution = weighted_term.weight * ranking::bm25_score(
                posting_list->term_frequency(doc_id),
                posting_list->document_frequency(),
                avgdl,
                documents_[doc_id].tokens.size(),
                documents_.size(),
                config_.bm25_k1,
                config_.bm25_b);

            if (contribution == 0.0)
            {
                continue;
            }

            highlight_terms.insert(weighted_term.term);
            add_unique(matched_terms, weighted_term.term);

            if (weighted_term.source == TermSource::Exact)
            {
                bm25_score += contribution;
            }
            else if (weighted_term.source == TermSource::Expansion)
            {
                expansion_score += contribution;
                add_unique(expansions, weighted_term.origin + " -> " + weighted_term.term);
            }
            else
            {
                fuzzy_score += contribution;
                std::ostringstream fuzzy_match;
                fuzzy_match << weighted_term.origin << " -> " << weighted_term.term
                            << " (d=" << weighted_term.edit_distance << ")";
                add_unique(fuzzy_matches, fuzzy_match.str());
            }
        }

        const double phrase_boost = phrase_boost_for_document(documents_[doc_id], phrases);
        const double score = bm25_score + expansion_score + fuzzy_score + phrase_boost;

        results.push_back({0, doc_id, score, bm25_score, expansion_score, fuzzy_score,
                           phrase_boost, documents_[doc_id].text,
                           make_snippet(documents_[doc_id], highlight_terms),
                           matched_terms, fuzzy_matches, expansions});
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

std::vector<std::string> SearchEngine::autocomplete(const std::string &prefix, std::size_t limit)
{
    const std::string normalized = parsing::normalize_token(prefix);
    if (normalized.empty())
    {
        return std::vector<std::string>();
    }

    analytics_.record_autocomplete(normalized);
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

void SearchEngine::print_analytics(std::ostream &out) const
{
    analytics_.print(out, config_.analytics_limit);
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

std::vector<SearchEngine::WeightedTerm> SearchEngine::build_weighted_terms(
    const std::vector<std::string> &terms) const
{
    std::vector<WeightedTerm> weighted_terms;
    std::unordered_set<std::string> seen;

    for (const std::string &term : terms)
    {
        weighted_terms.push_back({term, term, TermSource::Exact, 1.0, 0});
        seen.insert(term);

        const std::vector<std::string> expansion_terms = query_expander_.expand(term);
        for (const std::string &expanded : expansion_terms)
        {
            if (seen.insert(expanded).second)
            {
                weighted_terms.push_back(
                    {expanded, term, TermSource::Expansion, config_.expansion_weight, 0});
            }
        }

        if (index_.search_word(term) != nullptr)
        {
            continue;
        }

        const std::vector<utils::FuzzyMatch> fuzzy_matches = utils::find_fuzzy_matches(
            term, vocabulary_, config_.fuzzy_max_distance, config_.fuzzy_candidate_limit);

        for (const utils::FuzzyMatch &match : fuzzy_matches)
        {
            if (seen.insert(match.term).second)
            {
                const double distance_penalty =
                    1.0 - (static_cast<double>(match.distance) /
                           static_cast<double>(std::max(term.size(), match.term.size()) + 1));
                weighted_terms.push_back(
                    {match.term, term, TermSource::Fuzzy,
                     config_.fuzzy_weight * distance_penalty, match.distance});
            }
        }
    }

    return weighted_terms;
}

std::vector<std::vector<std::string> > SearchEngine::phrase_candidates(
    const std::vector<std::string> &terms) const
{
    std::vector<std::vector<std::string> > phrases;

    if (terms.size() > 1)
    {
        phrases.push_back(terms);
    }

    for (const std::string &term : terms)
    {
        const std::vector<std::string> expansion_terms = query_expander_.expand(term);
        if (expansion_terms.size() > 1)
        {
            phrases.push_back(expansion_terms);
        }
    }

    return phrases;
}

double SearchEngine::phrase_boost_for_document(
    const Document &document, const std::vector<std::vector<std::string> > &phrases) const
{
    double boost = 0.0;

    for (const std::vector<std::string> &phrase : phrases)
    {
        if (contains_phrase(document.tokens, phrase))
        {
            boost += config_.phrase_boost;
        }
    }

    return boost;
}

bool SearchEngine::contains_phrase(const std::vector<std::string> &tokens,
                                   const std::vector<std::string> &phrase) const
{
    if (phrase.empty() || phrase.size() > tokens.size())
    {
        return false;
    }

    for (std::size_t i = 0; i <= tokens.size() - phrase.size(); ++i)
    {
        bool matched = true;
        for (std::size_t j = 0; j < phrase.size(); ++j)
        {
            if (tokens[i + j] != phrase[j])
            {
                matched = false;
                break;
            }
        }

        if (matched)
        {
            return true;
        }
    }

    return false;
}

std::string SearchEngine::make_snippet(
    const Document &document, const std::unordered_set<std::string> &highlight_terms) const
{
    if (document.tokens.empty() || document.raw_terms.empty())
    {
        return document.text;
    }

    std::size_t first_match = document.raw_terms.size();

    for (std::size_t i = 0; i < document.raw_terms.size(); ++i)
    {
        if (highlight_terms.count(parsing::normalize_token(document.raw_terms[i])) != 0)
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

        if (highlight_terms.count(parsing::normalize_token(document.raw_terms[i])) != 0)
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

std::string SearchEngine::join_terms(const std::vector<std::string> &terms) const
{
    std::ostringstream joined;

    for (std::size_t i = 0; i < terms.size(); ++i)
    {
        if (i > 0)
        {
            joined << ' ';
        }

        joined << terms[i];
    }

    return joined.str();
}

void SearchEngine::add_unique(std::vector<std::string> &values, const std::string &value) const
{
    if (std::find(values.begin(), values.end(), value) == values.end())
    {
        values.push_back(value);
    }
}

} // namespace intellisearch
