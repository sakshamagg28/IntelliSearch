# IntelliSearch Architecture Notes

IntelliSearch currently focuses on the C++ lexical retrieval core:

- `src/parsing`: text normalization, tokenization, and command parsing helpers
- `src/core`: trie vocabulary index, posting lists, and search orchestration
- `src/ranking`: BM25 scoring utilities
- `src/main.cpp`: interactive CLI

Future API, frontend, and semantic retrieval layers should treat this search core as the reusable foundation rather than duplicating indexing or ranking logic.
