# IntelliSearch Architecture Notes

IntelliSearch currently focuses on the C++ lexical retrieval core:

- `src/parsing`: text normalization, tokenization, and query expansion
- `src/core`: trie vocabulary index, posting lists, search orchestration, and analytics
- `src/ranking`: BM25 scoring utilities
- `src/utils`: bounded edit-distance fuzzy matching
- `src/main.cpp`: interactive CLI

Ranking remains BM25-first. Query expansion and fuzzy matches add weighted BM25 contributions, while exact phrase matches add a configurable boost. `SearchResult` carries each contribution separately so the CLI can explain rankings without duplicating scoring logic.

Future API, frontend, and semantic retrieval layers should treat this search core as the reusable foundation rather than duplicating indexing or ranking logic.
