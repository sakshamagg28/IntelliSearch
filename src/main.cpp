#include "search_engine.h"
#include "tokenizer.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace
{

struct CliOptions
{
    std::string dataset_path;
    std::string expansion_dictionary_path;
    std::size_t top_k;
    bool show_help;
};

void print_usage()
{
    std::cout << "Usage: ./search_engine -i <dataset> [-k <top_k>] [-e <expansions_file>]\n"
              << "Commands:\n"
              << "  /search <terms...>       Rank documents using BM25\n"
              << "  /search --explain <...>  Show ranking explanations once\n"
              << "  /topk <number>           Change result count for this session\n"
              << "  /explain <on|off>        Toggle ranking explanations\n"
              << "  /suggest <prefix> [n]    Autocomplete indexed terms\n"
              << "  /analytics               Show query and autocomplete counts\n"
              << "  /df [term]               Print document frequency\n"
              << "  /tf <doc_id> <term>      Print term frequency\n"
              << "  /help                    Show commands\n"
              << "  /exit                    Quit IntelliSearch\n";
}

bool parse_options(int argc, char *argv[], CliOptions &options)
{
    options.top_k = 10;
    options.show_help = false;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg == "-i")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for -i.\n";
                return false;
            }

            options.dataset_path = argv[++i];
        }
        else if (arg == "-e" || arg == "--expansions")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for " << arg << ".\n";
                return false;
            }

            options.expansion_dictionary_path = argv[++i];
        }
        else if (arg == "-k")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Missing value for -k.\n";
                return false;
            }

            std::size_t parsed = 0;
            if (!intellisearch::parsing::parse_unsigned(argv[++i], parsed) || parsed == 0)
            {
                std::cerr << "Top-k must be a positive integer.\n";
                return false;
            }

            options.top_k = parsed;
        }
        else if (arg == "-h" || arg == "--help")
        {
            options.show_help = true;
            return true;
        }
        else
        {
            std::cerr << "Unknown option: " << arg << '\n';
            return false;
        }
    }

    if (!options.show_help && options.dataset_path.empty())
    {
        std::cerr << "Dataset path is required.\n";
        return false;
    }

    return true;
}

std::vector<std::string> tail_terms(const std::vector<std::string> &parts, std::size_t start)
{
    if (parts.size() <= start)
    {
        return std::vector<std::string>();
    }

    return std::vector<std::string>(parts.begin() + static_cast<std::ptrdiff_t>(start), parts.end());
}

void print_list(const std::string &label, const std::vector<std::string> &values)
{
    if (values.empty())
    {
        return;
    }

    std::cout << "      " << label << ": ";
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << ", ";
        }

        std::cout << values[i];
    }
    std::cout << '\n';
}

void print_results(const std::vector<intellisearch::SearchResult> &results, bool explain)
{
    if (results.empty())
    {
        std::cout << "No matching documents found.\n";
        return;
    }

    std::cout << "\nRank  Doc ID  Score       Snippet\n";
    std::cout << "----  ------  ----------  ----------------------------------------\n";

    for (const intellisearch::SearchResult &result : results)
    {
        std::cout << std::setw(4) << result.rank << "  "
                  << std::setw(6) << result.doc_id << "  "
                  << std::setw(10) << std::fixed << std::setprecision(5) << result.score << "  "
                  << result.snippet << '\n';

        if (explain)
        {
            std::cout << "      score = bm25(" << std::fixed << std::setprecision(5)
                      << result.bm25_score << ")"
                      << " + expansion(" << result.expansion_score << ")"
                      << " + fuzzy(" << result.fuzzy_score << ")"
                      << " + phrase(" << result.phrase_boost << ")\n";
            print_list("matched terms", result.matched_terms);
            print_list("expansions", result.expansions);
            print_list("fuzzy matches", result.fuzzy_matches);
        }
    }

    std::cout << '\n';
}

} // namespace

int main(int argc, char *argv[])
{
    CliOptions options;
    if (!parse_options(argc, argv, options))
    {
        print_usage();
        return 1;
    }

    if (options.show_help)
    {
        print_usage();
        return 0;
    }

    intellisearch::EngineConfig config;
    config.top_k = options.top_k;

    intellisearch::SearchEngine engine(config);
    std::string error;

    if (!options.expansion_dictionary_path.empty() &&
        !engine.load_expansion_dictionary(options.expansion_dictionary_path, error))
    {
        std::cerr << "Expansion dictionary failed: " << error << '\n';
        return 1;
    }

    if (!engine.load_documents(options.dataset_path, error))
    {
        std::cerr << "Indexing failed: " << error << '\n';
        return 1;
    }

    std::size_t current_top_k = options.top_k;
    bool explain_results = false;

    std::cout << "IntelliSearch index ready\n"
              << "Dataset: " << options.dataset_path << '\n'
              << "Expansion dictionary: "
              << (options.expansion_dictionary_path.empty() ? "built-in defaults" : options.expansion_dictionary_path)
              << '\n'
              << "Documents: " << engine.document_count() << '\n'
              << "Indexed terms: " << engine.total_terms() << '\n'
              << "Average document length: " << std::fixed << std::setprecision(2)
              << engine.average_document_length() << "\n\n";

    print_usage();

    std::string line;
    while (true)
    {
        std::cout << "\nsearch_engine> ";
        if (!std::getline(std::cin, line))
        {
            break;
        }

        line = intellisearch::parsing::trim(line);
        if (line.empty())
        {
            continue;
        }

        const std::vector<std::string> parts = intellisearch::parsing::split_command(line);
        const std::string command = parts[0];

        if (command == "/exit")
        {
            break;
        }

        if (command == "/help")
        {
            print_usage();
        }
        else if (command == "/topk")
        {
            if (parts.size() != 2)
            {
                std::cout << "Usage: /topk <positive_number>\n";
                continue;
            }

            std::size_t parsed = 0;
            if (!intellisearch::parsing::parse_unsigned(parts[1], parsed) || parsed == 0)
            {
                std::cout << "Top-k must be a positive integer.\n";
                continue;
            }

            current_top_k = parsed;
            std::cout << "Top-k set to " << current_top_k << ".\n";
        }
        else if (command == "/explain")
        {
            if (parts.size() != 2 || (parts[1] != "on" && parts[1] != "off"))
            {
                std::cout << "Usage: /explain <on|off>\n";
                continue;
            }

            explain_results = parts[1] == "on";
            std::cout << "Ranking explanations " << (explain_results ? "enabled" : "disabled") << ".\n";
        }
        else if (command == "/search")
        {
            std::vector<std::string> query_terms = tail_terms(parts, 1);
            bool explain_once = false;
            if (!query_terms.empty() && query_terms[0] == "--explain")
            {
                explain_once = true;
                query_terms.erase(query_terms.begin());
            }

            if (query_terms.empty())
            {
                std::cout << "Usage: /search <term> [term...]\n";
                continue;
            }

            print_results(engine.search(query_terms, current_top_k), explain_results || explain_once);
        }
        else if (command == "/suggest" || command == "/autocomplete")
        {
            if (parts.size() < 2 || parts.size() > 3)
            {
                std::cout << "Usage: /suggest <prefix> [limit]\n";
                continue;
            }

            std::size_t limit = engine.config().autocomplete_limit;
            if (parts.size() == 3 &&
                (!intellisearch::parsing::parse_unsigned(parts[2], limit) || limit == 0))
            {
                std::cout << "Suggestion limit must be a positive integer.\n";
                continue;
            }

            const std::vector<std::string> suggestions = engine.autocomplete(parts[1], limit);
            if (suggestions.empty())
            {
                std::cout << "No suggestions found.\n";
                continue;
            }

            for (const std::string &suggestion : suggestions)
            {
                std::cout << "  " << suggestion << '\n';
            }
        }
        else if (command == "/analytics")
        {
            if (parts.size() != 1)
            {
                std::cout << "Usage: /analytics\n";
                continue;
            }

            engine.print_analytics(std::cout);
        }
        else if (command == "/df")
        {
            if (parts.size() == 1)
            {
                engine.print_document_frequencies(std::cout);
            }
            else if (parts.size() == 2)
            {
                std::cout << parts[1] << "  " << engine.document_frequency(parts[1]) << '\n';
            }
            else
            {
                std::cout << "Usage: /df [term]\n";
            }
        }
        else if (command == "/tf")
        {
            if (parts.size() != 3)
            {
                std::cout << "Usage: /tf <doc_id> <term>\n";
                continue;
            }

            std::size_t doc_id = 0;
            if (!intellisearch::parsing::parse_unsigned(parts[1], doc_id))
            {
                std::cout << "Document id must be a non-negative integer.\n";
                continue;
            }

            std::cout << doc_id << "  " << parts[2] << "  "
                      << engine.term_frequency(doc_id, parts[2]) << '\n';
        }
        else
        {
            std::cout << "Unknown command. Type /help for available commands.\n";
        }
    }

    std::cout << "\nIntelliSearch stopped.\n";
    return 0;
}
