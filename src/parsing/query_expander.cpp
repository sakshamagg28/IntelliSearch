#include "query_expander.h"

#include "tokenizer.h"

#include <fstream>
#include <sstream>

namespace intellisearch
{
namespace parsing
{

namespace
{

std::string strip_comment(const std::string &line)
{
    const std::string::size_type comment = line.find('#');
    return comment == std::string::npos ? line : line.substr(0, comment);
}

} // namespace

QueryExpander::QueryExpander()
{
    set_expansion("dp", tokenize("dynamic programming"));
    set_expansion("os", tokenize("operating system"));
    set_expansion("dbms", tokenize("database management system"));
    set_expansion("ai", tokenize("artificial intelligence"));
    set_expansion("ml", tokenize("machine learning"));
}

bool QueryExpander::load_from_file(const std::string &path, std::string &error)
{
    std::ifstream input(path.c_str());
    if (!input)
    {
        error = "Unable to open expansion dictionary: " + path;
        return false;
    }

    std::string line;
    std::size_t line_number = 0;
    while (std::getline(input, line))
    {
        ++line_number;
        line = trim(strip_comment(line));
        if (line.empty())
        {
            continue;
        }

        std::string::size_type delimiter = line.find('=');
        if (delimiter == std::string::npos)
        {
            delimiter = line.find(':');
        }

        if (delimiter == std::string::npos)
        {
            std::ostringstream message;
            message << "Expansion dictionary line " << line_number
                    << " must use '<term>=<expansion terms>'.";
            error = message.str();
            return false;
        }

        const std::string key = normalize_token(trim(line.substr(0, delimiter)));
        const std::vector<std::string> values = tokenize(line.substr(delimiter + 1));

        if (key.empty() || values.empty())
        {
            std::ostringstream message;
            message << "Expansion dictionary line " << line_number
                    << " has an empty term or expansion.";
            error = message.str();
            return false;
        }

        set_expansion(key, values);
    }

    return true;
}

void QueryExpander::set_expansion(const std::string &term,
                                  const std::vector<std::string> &expansion_terms)
{
    const std::string normalized = normalize_token(term);
    if (normalized.empty() || expansion_terms.empty())
    {
        return;
    }

    std::vector<std::string> normalized_expansion;
    for (const std::string &term : expansion_terms)
    {
        const std::string normalized_term = normalize_token(term);
        if (!normalized_term.empty())
        {
            normalized_expansion.push_back(normalized_term);
        }
    }

    if (!normalized_expansion.empty())
    {
        expansions_[normalized] = normalized_expansion;
    }
}

std::vector<std::string> QueryExpander::expand(const std::string &term) const
{
    const std::string normalized = normalize_token(term);
    std::unordered_map<std::string, std::vector<std::string> >::const_iterator match =
        expansions_.find(normalized);

    if (match == expansions_.end())
    {
        return std::vector<std::string>();
    }

    return match->second;
}

} // namespace parsing
} // namespace intellisearch
