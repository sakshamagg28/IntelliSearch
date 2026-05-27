#include "search_engine.h"
#include "tokenizer.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

struct ApiOptions
{
    std::string dataset_path;
    std::string expansion_dictionary_path;
    std::string command;
    std::string query;
    std::size_t top_k = 10;
    std::size_t limit = 8;
    bool explain = false;
};

std::string json_escape(const std::string &value)
{
    std::ostringstream escaped;

    for (char ch : value)
    {
        switch (ch)
        {
        case '"':
            escaped << "\\\"";
            break;
        case '\\':
            escaped << "\\\\";
            break;
        case '\b':
            escaped << "\\b";
            break;
        case '\f':
            escaped << "\\f";
            break;
        case '\n':
            escaped << "\\n";
            break;
        case '\r':
            escaped << "\\r";
            break;
        case '\t':
            escaped << "\\t";
            break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20)
            {
                escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(ch))
                        << std::dec << std::setfill(' ');
            }
            else
            {
                escaped << ch;
            }
            break;
        }
    }

    return escaped.str();
}

void print_string_array(const std::vector<std::string> &values)
{
    std::cout << '[';
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << ',';
        }
        std::cout << '"' << json_escape(values[i]) << '"';
    }
    std::cout << ']';
}

void print_error(const std::string &message)
{
    std::cout << "{\"error\":\"" << json_escape(message) << "\"}\n";
}

bool parse_size(const std::string &value, std::size_t &parsed)
{
    return intellisearch::parsing::parse_unsigned(value, parsed) && parsed > 0;
}

bool parse_args(int argc, char *argv[], ApiOptions &options, std::string &error)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg == "--dataset")
        {
            if (i + 1 >= argc)
            {
                error = "Missing value for --dataset.";
                return false;
            }
            options.dataset_path = argv[++i];
        }
        else if (arg == "--expansions")
        {
            if (i + 1 >= argc)
            {
                error = "Missing value for --expansions.";
                return false;
            }
            options.expansion_dictionary_path = argv[++i];
        }
        else if (arg == "search" || arg == "autocomplete" || arg == "health")
        {
            options.command = arg;
        }
        else if (arg == "--query" || arg == "--q")
        {
            if (i + 1 >= argc)
            {
                error = "Missing query value.";
                return false;
            }
            options.query = argv[++i];
        }
        else if (arg == "--top-k")
        {
            if (i + 1 >= argc || !parse_size(argv[i + 1], options.top_k))
            {
                error = "--top-k must be a positive integer.";
                return false;
            }
            ++i;
        }
        else if (arg == "--limit")
        {
            if (i + 1 >= argc || !parse_size(argv[i + 1], options.limit))
            {
                error = "--limit must be a positive integer.";
                return false;
            }
            ++i;
        }
        else if (arg == "--explain")
        {
            options.explain = true;
        }
        else
        {
            error = "Unknown argument: " + arg;
            return false;
        }
    }

    if (options.command.empty())
    {
        error = "Command is required.";
        return false;
    }

    if (options.command != "health" && options.dataset_path.empty())
    {
        error = "--dataset is required.";
        return false;
    }

    if ((options.command == "search" || options.command == "autocomplete") && options.query.empty())
    {
        error = "--query is required.";
        return false;
    }

    return true;
}

bool load_engine(const ApiOptions &options, intellisearch::SearchEngine &engine, std::string &error)
{
    if (!options.expansion_dictionary_path.empty() &&
        !engine.load_expansion_dictionary(options.expansion_dictionary_path, error))
    {
        return false;
    }

    return engine.load_documents(options.dataset_path, error);
}

void print_search_response(intellisearch::SearchEngine &engine, const ApiOptions &options)
{
    const std::vector<std::string> query_terms = intellisearch::parsing::tokenize(options.query);
    const std::vector<intellisearch::SearchResult> results = engine.search(query_terms, options.top_k);

    std::cout << "{\"query\":\"" << json_escape(options.query) << "\",";
    std::cout << "\"top_k\":" << options.top_k << ',';
    std::cout << "\"explain\":" << (options.explain ? "true" : "false") << ',';
    std::cout << "\"results\":[";

    for (std::size_t i = 0; i < results.size(); ++i)
    {
        const intellisearch::SearchResult &result = results[i];
        if (i > 0)
        {
            std::cout << ',';
        }

        std::cout << '{'
                  << "\"rank\":" << result.rank << ','
                  << "\"doc_id\":" << result.doc_id << ','
                  << "\"document\":\"" << json_escape(result.document) << "\","
                  << "\"score\":" << std::fixed << std::setprecision(6) << result.score << ','
                  << "\"snippet\":\"" << json_escape(result.snippet) << '"';

        if (options.explain)
        {
            std::cout << ",\"ranking_details\":{"
                      << "\"bm25_score\":" << result.bm25_score << ','
                      << "\"expansion_score\":" << result.expansion_score << ','
                      << "\"fuzzy_score\":" << result.fuzzy_score << ','
                      << "\"phrase_boost\":" << result.phrase_boost << ','
                      << "\"matched_terms\":";
            print_string_array(result.matched_terms);
            std::cout << ",\"expansions\":";
            print_string_array(result.expansions);
            std::cout << ",\"fuzzy_matches\":";
            print_string_array(result.fuzzy_matches);
            std::cout << '}';
        }

        std::cout << '}';
    }

    std::cout << "]}\n";
}

void print_autocomplete_response(intellisearch::SearchEngine &engine, const ApiOptions &options)
{
    const std::vector<std::string> suggestions = engine.autocomplete(options.query, options.limit);

    std::cout << "{\"query\":\"" << json_escape(options.query) << "\",\"suggestions\":";
    print_string_array(suggestions);
    std::cout << "}\n";
}

} // namespace

int main(int argc, char *argv[])
{
    ApiOptions options;
    std::string error;

    if (!parse_args(argc, argv, options, error))
    {
        print_error(error);
        return 1;
    }

    if (options.command == "health")
    {
        std::cout << "{\"status\":\"ok\",\"engine\":\"IntelliSearch\"}\n";
        return 0;
    }

    intellisearch::EngineConfig config;
    config.top_k = options.top_k;
    intellisearch::SearchEngine engine(config);

    if (!load_engine(options, engine, error))
    {
        print_error(error);
        return 1;
    }

    if (options.command == "search")
    {
        print_search_response(engine, options);
    }
    else if (options.command == "autocomplete")
    {
        print_autocomplete_response(engine, options);
    }
    else
    {
        print_error("Unsupported command: " + options.command);
        return 1;
    }

    return 0;
}
