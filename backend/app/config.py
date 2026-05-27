from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


def _resolve_path(project_root: Path, value: str) -> Path:
    path = Path(value)
    return path if path.is_absolute() else project_root / path


@dataclass(frozen=True)
class Settings:
    project_root: Path
    core_binary: Path
    dataset_path: Path
    expansion_dictionary_path: Optional[Path]
    command_timeout_seconds: float

    @classmethod
    def from_env(cls) -> "Settings":
        project_root = Path(__file__).resolve().parents[2]

        core_binary = _resolve_path(
            project_root,
            os.getenv("INTELLISEARCH_CORE_BINARY", "search_engine_api"),
        )
        dataset_path = _resolve_path(
            project_root,
            os.getenv("INTELLISEARCH_DATASET", "datasets/smallDataset.txt"),
        )
        expansion_path = os.getenv("INTELLISEARCH_EXPANSIONS")

        return cls(
            project_root=project_root,
            core_binary=core_binary,
            dataset_path=dataset_path,
            expansion_dictionary_path=_resolve_path(project_root, expansion_path)
            if expansion_path
            else None,
            command_timeout_seconds=float(os.getenv("INTELLISEARCH_COMMAND_TIMEOUT", "15")),
        )
