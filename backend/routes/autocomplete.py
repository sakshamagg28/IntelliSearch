from __future__ import annotations

from fastapi import APIRouter, HTTPException, Query, Request, status

from backend.models.autocomplete import AutocompleteResponse
from backend.services.search_service import SearchServiceError

router = APIRouter(tags=["autocomplete"])


@router.get("/autocomplete", response_model=AutocompleteResponse)
def autocomplete(
    request: Request,
    q: str = Query(..., min_length=1),
    limit: int = Query(8, ge=1, le=50),
) -> AutocompleteResponse:
    try:
        response = request.app.state.search_service.autocomplete(query=q, limit=limit)
    except SearchServiceError as exc:
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail=str(exc),
        ) from exc

    request.app.state.analytics_service.record_autocomplete(q)
    return AutocompleteResponse(**response)
