from __future__ import annotations

import json
import subprocess
from pathlib import Path
from typing import Any, Dict, List, Optional

from backend.app.config import Settings


class SearchServiceError(RuntimeError):
    pass


class SearchService:
    def __init__(self, settings: Settings) -> None:
        self._settings = settings

    def search(self, query: str, top_k: int, explain: bool) -> Dict[str, Any]:
        args = [
            "search",
            "--query",
            query,
            "--top-k",
            str(top_k),
        ]

        if explain:
            args.append("--explain")

        return self._run_engine(args)

    def autocomplete(self, query: str, limit: int) -> Dict[str, Any]:
        return self._run_engine(["autocomplete", "--q", query, "--limit", str(limit)])

    def health(self) -> Dict[str, Any]:
        binary_exists = self._settings.core_binary.exists()
        dataset_exists = self._settings.dataset_path.exists()

        return {
            "status": "ok" if binary_exists and dataset_exists else "degraded",
            "engine": "IntelliSearch",
            "core_binary": str(self._settings.core_binary),
            "dataset": str(self._settings.dataset_path),
            "core_binary_exists": binary_exists,
            "dataset_exists": dataset_exists,
        }

    def _run_engine(self, args: List[str]) -> Dict[str, Any]:
        self._validate_runtime()

        command = [
            str(self._settings.core_binary),
            "--dataset",
            str(self._settings.dataset_path),
        ]

        if self._settings.expansion_dictionary_path is not None:
            command.extend(["--expansions", str(self._settings.expansion_dictionary_path)])

        command.extend(args)

        try:
            completed = subprocess.run(
                command,
                cwd=str(self._settings.project_root),
                check=False,
                capture_output=True,
                text=True,
                timeout=self._settings.command_timeout_seconds,
            )
        except subprocess.TimeoutExpired as exc:
            raise SearchServiceError("C++ search engine command timed out.") from exc
        except OSError as exc:
            raise SearchServiceError(f"Unable to execute C++ search engine: {exc}") from exc

        payload = self._parse_json(completed.stdout)

        if completed.returncode != 0:
            message = payload.get("error") if isinstance(payload, dict) else None
            raise SearchServiceError(message or completed.stderr or "C++ search engine failed.")

        if not isinstance(payload, dict):
            raise SearchServiceError("C++ search engine returned an invalid payload.")

        return payload

    def _validate_runtime(self) -> None:
        if not self._settings.core_binary.exists():
            raise SearchServiceError(
                f"C++ adapter binary not found at {self._settings.core_binary}. Run `make` first."
            )

        if not self._settings.dataset_path.exists():
            raise SearchServiceError(f"Dataset not found at {self._settings.dataset_path}.")

        expansion_path: Optional[Path] = self._settings.expansion_dictionary_path
        if expansion_path is not None and not expansion_path.exists():
            raise SearchServiceError(f"Expansion dictionary not found at {expansion_path}.")

    def _parse_json(self, output: str) -> Any:
        try:
            return json.loads(output)
        except json.JSONDecodeError as exc:
            raise SearchServiceError("C++ search engine returned malformed JSON.") from exc
