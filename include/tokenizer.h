#ifndef INTELLISEARCH_TOKENIZER_H
#define INTELLISEARCH_TOKENIZER_H

#include <string>
#include <vector>

namespace intellisearch
{
namespace parsing
{

std::string normalize_token(const std::string &token);
std::vector<std::string> tokenize(const std::string &text);
std::string trim(const std::string &text);
std::vector<std::string> split_command(const std::string &line);
bool parse_unsigned(const std::string &text, std::size_t &value);

} // namespace parsing
} // namespace intellisearch

#endif
