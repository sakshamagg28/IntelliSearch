from __future__ import annotations

from fastapi import APIRouter, HTTPException, Request, status

from backend.models.search import SearchRequest, SearchResponse
from backend.services.search_service import SearchServiceError

router = APIRouter(tags=["search"])


@router.post("/search", response_model=SearchResponse)
def search_documents(payload: SearchRequest, request: Request) -> SearchResponse:
    try:
        response = request.app.state.search_service.search(
            query=payload.query,
            top_k=payload.top_k,
            explain=payload.explain,
        )
    except SearchServiceError as exc:
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail=str(exc),
        ) from exc

    request.app.state.analytics_service.record_query(payload.query)
    return SearchResponse(**response)
