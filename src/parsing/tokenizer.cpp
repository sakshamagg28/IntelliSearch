#include "tokenizer.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>

namespace intellisearch
{
namespace parsing
{

std::string normalize_token(const std::string &token)
{
    std::string normalized;

    for (char ch : token)
    {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch))
        {
            normalized.push_back(static_cast<char>(std::tolower(uch)));
        }
    }

    return normalized;
}

std::vector<std::string> tokenize(const std::string &text)
{
    std::vector<std::string> tokens;
    std::string current;

    for (char ch : text)
    {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (std::isalnum(uch))
        {
            current.push_back(static_cast<char>(std::tolower(uch)));
        }
        else if (!current.empty())
        {
            tokens.push_back(current);
            current.clear();
        }
    }

    if (!current.empty())
    {
        tokens.push_back(current);
    }

    return tokens;
}

std::string trim(const std::string &text)
{
    const std::string whitespace = " \t\n\r\f\v";
    const std::string::size_type first = text.find_first_not_of(whitespace);

    if (first == std::string::npos)
    {
        return "";
    }

    const std::string::size_type last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

std::vector<std::string> split_command(const std::string &line)
{
    std::vector<std::string> parts;
    std::istringstream stream(line);
    std::string part;

    while (stream >> part)
    {
        parts.push_back(part);
    }

    return parts;
}

bool parse_unsigned(const std::string &text, std::size_t &value)
{
    if (text.empty())
    {
        return false;
    }

    std::size_t parsed = 0;
    for (char ch : text)
    {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (!std::isdigit(uch))
        {
            return false;
        }

        const std::size_t digit = static_cast<std::size_t>(ch - '0');
        if (parsed > (std::numeric_limits<std::size_t>::max() - digit) / 10)
        {
            return false;
        }

        parsed = parsed * 10 + digit;
    }

    value = parsed;
    return true;
}

} // namespace parsing
} // namespace intellisearch
