# IntelliSearch Architecture Notes

IntelliSearch currently has two layers: the C++ lexical retrieval core and a thin FastAPI backend.

- `src/parsing`: text normalization, tokenization, and query expansion
- `src/core`: trie vocabulary index, posting lists, search orchestration, and analytics
- `src/ranking`: BM25 scoring utilities
- `src/utils`: bounded edit-distance fuzzy matching
- `src/main.cpp`: interactive CLI
- `src/api_runner.cpp`: JSON adapter used by the backend
- `backend/routes`: REST endpoint modules
- `backend/services`: subprocess adapter for the C++ engine and in-process API analytics
- `backend/models`: Pydantic schemas for request and response payloads

Ranking remains BM25-first. Query expansion and fuzzy matches add weighted BM25 contributions, while exact phrase matches add a configurable boost. `SearchResult` carries each contribution separately so the CLI can explain rankings without duplicating scoring logic.

The backend invokes `search_engine_api` with explicit arguments and receives JSON. This keeps the C++ engine reusable and avoids parsing the human CLI output. Future frontend and semantic retrieval layers should treat the C++ search core and backend API as the reusable foundation rather than duplicating indexing or ranking logic.
