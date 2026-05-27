# IntelliSearch

IntelliSearch is a systems-oriented C++ search engine foundation built around classic information retrieval data structures: an inverted index, sorted posting lists, BM25 ranking, trie-backed autocomplete, and a robust command-line search interface.

The project began as an academic search prototype and has been refactored into a cleaner, portfolio-ready codebase while preserving the original core idea: index a document collection, retrieve candidate documents through posting lists, score them with BM25, and return the top-k ranked results.

## Architecture

```text
IntelliSearch/
├── include/                 Public C++ headers
├── src/
│   ├── core/                Trie, posting lists, search engine orchestration
│   ├── parsing/             Query/document tokenization and CLI parsing helpers
│   ├── ranking/             BM25 relevance scoring
│   ├── utils/               Reserved for small shared utilities
│   └── main.cpp             Interactive CLI entry point
├── datasets/                Example document collections
├── queries/                 Sample query workloads
├── results/                 Historical expected outputs
├── docs/                    Design notes and future documentation
├── tests/                   Future regression and unit tests
├── Makefile
└── README.md
```

## Indexing Pipeline

1. The CLI loads a dataset where each line is one document:

   ```text
   0 Document text goes here
   1 Another document appears here
   ```

2. Document IDs are validated to be numeric, contiguous, and zero-based.
3. Text is tokenized with punctuation handling and case normalization.
4. Each normalized term is inserted into a trie.
5. Terminal trie nodes own posting lists that track:
   - which documents contain the term
   - how many times the term appears in each document
6. Document length statistics are retained for BM25 scoring.

## Inverted Index

The inverted index maps terms to posting lists:

```text
term -> [(doc_id, term_frequency), ...]
```

Posting lists are kept sorted by document ID, which makes term-frequency lookup predictable and keeps the implementation lightweight. This is the central structure used by `/search`, `/df`, and `/tf`.

## BM25 Ranking

IntelliSearch ranks documents with Okapi BM25, a standard lexical relevance function used in information retrieval systems.

For each query term, BM25 considers:

- term frequency in the document
- document frequency across the collection
- total document count
- document length
- average document length
- configurable `k1` and `b` constants

The engine sums per-term BM25 scores for candidate documents and returns the configured top-k results.

## Trie Autocomplete

The trie stores indexed vocabulary character-by-character. This supports fast prefix lookup:

```text
/suggest alg 5
```

Example output:

```text
  algebra
  algorithm
  algorithms
```

Suggestions are generated directly from indexed terms, so autocomplete reflects the current dataset.

## CLI Commands

Build the project:

```bash
make
```

Run against the small dataset:

```bash
./search_engine -i datasets/smallDataset.txt -k 5
```

Interactive commands:

```text
/search <terms...>       Rank documents using BM25
/topk <number>           Change result count for this session
/suggest <prefix> [n]    Show trie autocomplete suggestions
/df [term]               Print document frequency
/tf <doc_id> <term>      Print term frequency
/help                    Show available commands
/exit                    Quit IntelliSearch
```

Example:

```text
search_engine> /search data structures

Rank  Doc ID  Score       Snippet
----  ------  ----------  ----------------------------------------
   1      55     2.05571  ... Hot 100 chart now incorporates [data] from sales ...
```

Matched query terms are highlighted in snippets with square brackets.

## Engineering Highlights

- Refactored from a flat coursework layout into `src/`, `include/`, `ranking/`, and `parsing/` modules.
- Preserves the original trie plus posting-list indexing model.
- Uses RAII in posting lists to reduce manual memory-management risk.
- Separates tokenization, ranking, indexing, and CLI responsibilities.
- Adds robust command validation to prevent malformed input crashes.
- Supports configurable top-k both at startup and during an interactive session.
- Adds trie-backed autocomplete without introducing external dependencies.
- Keeps the implementation lightweight and suitable for future systems-level extensions.

## Make Targets

```bash
make        # Build search_engine
make run    # Build and run with datasets/smallDataset.txt
make clean  # Remove build artifacts
```

## Roadmap

Planned future phases:

- Add focused unit tests for tokenization, posting lists, trie lookup, and BM25.
- Add query operators such as phrase queries, boolean filters, and field-aware search.
- Improve ranking with score normalization and better handling for very common terms.
- Add fuzzy matching with edit distance or Levenshtein automata.
- Add document ingestion helpers for richer file formats.
- Later, introduce API and frontend layers around this C++ search core.
- Later, add semantic retrieval and hybrid lexical/vector ranking.

The current codebase intentionally remains a focused C++ search engine foundation. Backend APIs, frontend UI, databases, and semantic search are future phases rather than current dependencies.
