from __future__ import annotations

from typing import List, Optional

from pydantic import BaseModel, Field


class SearchRequest(BaseModel):
    query: str = Field(..., min_length=1)
    top_k: int = Field(10, ge=1, le=100)
    explain: bool = False


class RankingDetails(BaseModel):
    bm25_score: float
    expansion_score: float
    fuzzy_score: float
    phrase_boost: float
    matched_terms: List[str]
    expansions: List[str]
    fuzzy_matches: List[str]


class SearchResult(BaseModel):
    rank: int
    doc_id: int
    document: str
    score: float
    snippet: str
    ranking_details: Optional[RankingDetails] = None


class SearchResponse(BaseModel):
    query: str
    top_k: int
    explain: bool
    results: List[SearchResult]
