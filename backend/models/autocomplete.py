from __future__ import annotations

from typing import List

from pydantic import BaseModel


class AutocompleteResponse(BaseModel):
    query: str
    suggestions: List[str]
