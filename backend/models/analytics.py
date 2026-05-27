from __future__ import annotations

from typing import List

from pydantic import BaseModel


class AnalyticsEntry(BaseModel):
    value: str
    count: int


class AnalyticsResponse(BaseModel):
    top_queries: List[AnalyticsEntry]
    autocomplete_usage: List[AnalyticsEntry]
