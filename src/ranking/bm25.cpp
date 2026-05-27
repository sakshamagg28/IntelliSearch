#include "bm25.h"

#include <cmath>

namespace intellisearch
{
namespace ranking
{

double inverse_document_frequency(std::size_t total_documents, std::size_t document_frequency)
{
    if (document_frequency == 0 || total_documents == 0)
    {
        return 0.0;
    }

    const double numerator = static_cast<double>(total_documents) - static_cast<double>(document_frequency) + 0.5;
    const double denominator = static_cast<double>(document_frequency) + 0.5;
    return std::log10(numerator / denominator);
}

double bm25_score(std::size_t term_frequency, std::size_t document_frequency,
                  double average_document_length, std::size_t document_length,
                  std::size_t total_documents, double k1, double b)
{
    if (term_frequency == 0 || document_frequency == 0 || average_document_length == 0.0)
    {
        return 0.0;
    }

    const double tf = static_cast<double>(term_frequency);
    const double length = static_cast<double>(document_length);
    const double idf = inverse_document_frequency(total_documents, document_frequency);
    const double denominator = tf + k1 * (1.0 - b + b * (length / average_document_length));
    return idf * ((tf * (k1 + 1.0)) / denominator);
}

} // namespace ranking
} // namespace intellisearch
