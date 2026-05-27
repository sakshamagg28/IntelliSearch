# IntelliSearch

IntelliSearch is a systems-oriented C++ search platform built around classic information retrieval data structures: an inverted index, sorted posting lists, BM25 ranking, trie-backed autocomplete, fuzzy matching, query expansion, phrase-aware ranking, a robust command-line interface, and a FastAPI backend.

The project began as an academic search prototype and has been refactored into a cleaner, portfolio-ready codebase while preserving the original core idea: index a document collection, retrieve candidate documents through posting lists, score them with BM25, and return the top-k ranked results.

## Architecture

```text
IntelliSearch/
├── backend/
│   ├── app/                  FastAPI app factory and runtime config
│   ├── models/               Pydantic request/response schemas
│   ├── routes/               REST endpoint modules
│   ├── services/             C++ adapter and analytics services
│   ├── main.py               ASGI entry point
│   └── requirements.txt
├── include/                 Public C++ headers
├── src/
│   ├── core/                Trie, posting lists, search engine orchestration
│   ├── parsing/             Tokenization and query expansion
│   ├── ranking/             BM25 relevance scoring
│   ├── utils/               Fuzzy matching helpers
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
6. Document length statistics and vocabulary terms are retained for BM25 and fuzzy matching.

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

The engine uses BM25 as the primary score, then adds smaller contributions from query expansion, fuzzy matches, and exact phrase boosts. Ranking explanations can show how each component affected the final score.

## Query Intelligence

IntelliSearch adds lightweight query intelligence on top of lexical retrieval:

- **Fuzzy search:** misspelled terms are corrected against indexed vocabulary with bounded edit distance. For example, `spearrs` can match `spears`.
- **Query expansion:** acronyms and aliases expand into related terms. Built-in examples include `dp -> dynamic programming`, `os -> operating system`, and `dbms -> database management system`.
- **Configurable dictionaries:** pass `-e <file>` or `--expansions <file>` to load extra expansions. Each line uses `term=expanded words`; `#` comments are ignored.
- **Phrase boosting:** multi-term queries receive an additional score when a document contains the exact normalized phrase, while BM25 remains the primary ranking signal.

Expansion dictionary example:

```text
ir=information retrieval
nlp=natural language processing
```

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

Autocomplete prefix usage is tracked by the analytics module.

## Search Analytics

The CLI and backend both track lightweight in-process analytics:

- normalized search query frequency
- autocomplete prefix frequency

Use `/analytics` in the CLI or `GET /analytics` in the backend to inspect the most frequent searches and autocomplete prefixes in the current run.

## Backend API

The FastAPI backend exposes the C++ IntelliSearch engine through REST endpoints. It uses a lightweight C++ JSON adapter binary, `search_engine_api`, so the backend receives structured engine output without parsing CLI text.

Build the C++ CLI and API adapter:

```bash
make
```

Install backend dependencies:

```bash
python3 -m pip install -r backend/requirements.txt
```

Start the backend:

```bash
python3 -m uvicorn backend.main:app --reload --host 127.0.0.1 --port 8000
```

Runtime configuration is provided with environment variables:

```bash
export INTELLISEARCH_DATASET=datasets/smallDataset.txt
export INTELLISEARCH_EXPANSIONS=docs/expansions.example.txt
export INTELLISEARCH_CORE_BINARY=./search_engine_api
```

If omitted, the backend defaults to `datasets/smallDataset.txt` and `./search_engine_api`.

### API Endpoints

```text
GET  /health
POST /search
GET  /autocomplete?q=<prefix>&limit=<n>
GET  /analytics
```

Health check:

```bash
curl http://127.0.0.1:8000/health
```

Search with explanations:

```bash
curl -X POST http://127.0.0.1:8000/search \
  -H "Content-Type: application/json" \
  -d '{"query":"spearrs","top_k":3,"explain":true}'
```

Autocomplete:

```bash
curl "http://127.0.0.1:8000/autocomplete?q=mus&limit=5"
```

Analytics:

```bash
curl "http://127.0.0.1:8000/analytics?limit=10"
```

Search responses include ranked documents, scores, snippets, and optional ranking details:

```json
{
  "query": "spearrs",
  "top_k": 1,
  "explain": true,
  "results": [
    {
      "rank": 1,
      "doc_id": 0,
      "document": "Spears seeks aborted tour payment ...",
      "score": 1.639902,
      "snippet": "[Spears] seeks aborted tour payment Singer Britney [Spears] is ...",
      "ranking_details": {
        "bm25_score": 0.0,
        "expansion_score": 0.0,
        "fuzzy_score": 1.639902,
        "phrase_boost": 0.0,
        "matched_terms": ["spears"],
        "expansions": [],
        "fuzzy_matches": ["spearrs -> spears (d=1)"]
      }
    }
  ]
}
```

## CLI Commands

Build the project:

```bash
make
```

Run against the small dataset:

```bash
./search_engine -i datasets/smallDataset.txt -k 5
```

Run with a custom expansion dictionary:

```bash
./search_engine -i datasets/smallDataset.txt -k 5 -e docs/expansions.example.txt
```

Interactive commands:

```text
/search <terms...>       Rank documents using BM25
/search --explain <...>  Show ranking explanations once
/topk <number>           Change result count for this session
/explain <on|off>        Toggle ranking explanations
/suggest <prefix> [n]    Show trie autocomplete suggestions
/analytics               Show query and autocomplete counts
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

Explanation example:

```text
search_engine> /search --explain spearrs

Rank  Doc ID  Score       Snippet
----  ------  ----------  ----------------------------------------
   1       0     1.63990  [Spears] seeks aborted tour payment Singer Britney [Spears] is ...
      score = bm25(0.00000) + expansion(0.00000) + fuzzy(1.63990) + phrase(0.00000)
      matched terms: spears
      fuzzy matches: spearrs -> spears (d=1)
```

## Engineering Highlights

- Refactored from a flat coursework layout into `src/`, `include/`, `ranking/`, and `parsing/` modules.
- Preserves the original trie plus posting-list indexing model.
- Uses RAII in posting lists to reduce manual memory-management risk.
- Separates tokenization, ranking, indexing, and CLI responsibilities.
- Adds typo-tolerant retrieval with bounded edit distance.
- Adds configurable query expansion for acronyms and synonyms.
- Adds exact phrase boosting without replacing BM25.
- Tracks lightweight search and autocomplete analytics.
- Provides ranking explanations for debugging retrieval behavior.
- Adds robust command validation to prevent malformed input crashes.
- Supports configurable top-k both at startup and during an interactive session.
- Adds trie-backed autocomplete without introducing external dependencies.
- Keeps the implementation lightweight and suitable for future systems-level extensions.

## Make Targets

```bash
make        # Build search_engine and search_engine_api
make run    # Build and run with datasets/smallDataset.txt
make clean  # Remove build artifacts
```

## Roadmap

Planned future phases:

- Add focused unit tests for tokenization, posting lists, trie lookup, and BM25.
- Add focused unit tests for fuzzy matching, expansion, analytics, and explanation output.
- Add query operators such as boolean filters and field-aware search.
- Improve ranking with score normalization and better handling for very common terms.
- Optimize fuzzy matching with trie traversal or Levenshtein automata for larger vocabularies.
- Add document ingestion helpers for richer file formats.
- Later, introduce a frontend around the backend API.
- Later, add semantic retrieval and hybrid lexical/vector ranking.

The current codebase intentionally remains a focused C++ search engine with a thin backend API layer. Frontend UI, databases, authentication, and semantic search are future phases rather than current dependencies.
