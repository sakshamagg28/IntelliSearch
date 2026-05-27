from __future__ import annotations

from fastapi import APIRouter, Query, Request

from backend.models.analytics import AnalyticsResponse

router = APIRouter(tags=["analytics"])


@router.get("/analytics", response_model=AnalyticsResponse)
def analytics(
    request: Request,
    limit: int = Query(10, ge=1, le=100),
) -> AnalyticsResponse:
    return AnalyticsResponse(**request.app.state.analytics_service.snapshot(limit=limit))
