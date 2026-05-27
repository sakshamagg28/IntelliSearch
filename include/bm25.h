#ifndef INTELLISEARCH_BM25_H
#define INTELLISEARCH_BM25_H

#include <cstddef>

namespace intellisearch
{
namespace ranking
{

double inverse_document_frequency(std::size_t total_documents, std::size_t document_frequency);
double bm25_score(std::size_t term_frequency, std::size_t document_frequency,
                  double average_document_length, std::size_t document_length,
                  std::size_t total_documents, double k1, double b);

} // namespace ranking
} // namespace intellisearch

#endif
