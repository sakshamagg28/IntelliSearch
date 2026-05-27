from __future__ import annotations

from fastapi import APIRouter, Request

from backend.models.health import HealthResponse

router = APIRouter(tags=["health"])


@router.get("/health", response_model=HealthResponse)
def health(request: Request) -> HealthResponse:
    return HealthResponse(**request.app.state.search_service.health())
