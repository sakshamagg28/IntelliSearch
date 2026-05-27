from __future__ import annotations

from typing import Optional

from fastapi import FastAPI

from backend.app.config import Settings
from backend.routes import analytics, autocomplete, health, search
from backend.services.analytics_service import AnalyticsService
from backend.services.search_service import SearchService


def create_app(settings: Optional[Settings] = None) -> FastAPI:
    app = FastAPI(
        title="IntelliSearch API",
        description="REST API layer for the IntelliSearch C++ retrieval engine.",
        version="0.1.0",
    )

    app.state.settings = settings or Settings.from_env()
    app.state.search_service = SearchService(app.state.settings)
    app.state.analytics_service = AnalyticsService()

    app.include_router(health.router)
    app.include_router(search.router)
    app.include_router(autocomplete.router)
    app.include_router(analytics.router)

    return app
