from __future__ import annotations

from pydantic import BaseModel


class HealthResponse(BaseModel):
    status: str
    engine: str
    core_binary: str
    dataset: str
    core_binary_exists: bool
    dataset_exists: bool
