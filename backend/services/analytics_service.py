from __future__ import annotations

from collections import Counter
from typing import Dict, List, Union


class AnalyticsService:
    def __init__(self) -> None:
        self._query_counts: Counter[str] = Counter()
        self._autocomplete_counts: Counter[str] = Counter()

    def record_query(self, query: str) -> None:
        normalized = self._normalize(query)
        if normalized:
            self._query_counts[normalized] += 1

    def record_autocomplete(self, prefix: str) -> None:
        normalized = self._normalize(prefix)
        if normalized:
            self._autocomplete_counts[normalized] += 1

    def snapshot(self, limit: int = 10) -> Dict[str, List[Dict[str, Union[int, str]]]]:
        return {
            "top_queries": self._top_entries(self._query_counts, limit),
            "autocomplete_usage": self._top_entries(self._autocomplete_counts, limit),
        }

    def _top_entries(self, counts: Counter[str], limit: int) -> List[Dict[str, Union[int, str]]]:
        return [
            {"value": value, "count": count}
            for value, count in sorted(counts.items(), key=lambda item: (-item[1], item[0]))[:limit]
        ]

    def _normalize(self, value: str) -> str:
        return " ".join(value.strip().lower().split())
