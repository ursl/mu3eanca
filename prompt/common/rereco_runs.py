"""Parse certification run lists and expand rereco task templates."""

from __future__ import annotations

import os
import re
from pathlib import Path
from typing import Any, Mapping


def parse_runs_file(path: str | Path) -> list[int]:
    """Read comma-separated run numbers from a .runs certification file."""
    text = Path(path).read_text()
    text = text.replace("{", "").replace("}", "")
    runs: list[int] = []
    seen: set[int] = set()
    for part in text.split(","):
        token = part.strip()
        if not token:
            continue
        if not re.fullmatch(r"\d+", token):
            raise ValueError(f"invalid run token {token!r} in {path}")
        run = int(token)
        if run not in seen:
            runs.append(run)
            seen.add(run)
    if not runs:
        raise ValueError(f"no run numbers found in {path}")
    return runs


def filter_runs(
    runs: list[int],
    min_run: int = 0,
    max_run: int = 0,
) -> list[int]:
    """Keep runs with min_run <= run <= max_run (0 bound = no limit)."""
    out = runs
    if min_run > 0:
        out = [r for r in out if r >= min_run]
    if max_run > 0:
        out = [r for r in out if r <= max_run]
    return out


def _fmt_template(value: str, run: int) -> str:
    return value.replace("{run:05d}", f"{run:05d}").replace("{run}", str(run))


def _template_field_key(key: str) -> str:
    if key.endswith("_tpl"):
        return key[:-4]
    return key


def _expand_task_template(tpl: Mapping[str, Any], run: int) -> dict[str, Any]:
    action = tpl.get("action", "trirec")
    task: dict[str, Any] = {"action": action}
    id_tpl = tpl.get("id_tpl") or f"run{{run}}-{action}"
    task["id"] = _fmt_template(id_tpl, run)

    for key, val in tpl.items():
        if key in ("action", "id_tpl"):
            continue
        out_key = _template_field_key(key)
        if isinstance(val, str):
            task[out_key] = _fmt_template(val, run)
        else:
            task[out_key] = val

    if action == "trirec" and "run_id" not in task:
        task["run_id"] = run
    return task


def expand_tasks_from_run_list(
    templates: list[Mapping[str, Any]],
    runs: list[int],
) -> list[dict[str, Any]]:
    tasks: list[dict[str, Any]] = []
    for run in runs:
        for tpl in templates:
            tasks.append(_expand_task_template(tpl, run))
    return tasks


def resolve_run_list_path(run_list_file: str, base: Path) -> Path:
    path = Path(run_list_file)
    if not path.is_absolute():
        path = base / path
    return path.resolve()


def load_rereco_tasks(config: Mapping[str, Any], base: Path) -> list[dict[str, Any]]:
    """Build task list from run-list templates or explicit rereco_tasks/rereco_jobs."""
    run_list_file = (config.get("run_list_file") or "").strip()
    templates = config.get("rereco_task_templates") or []

    if run_list_file:
        if not templates:
            raise ValueError(
                "run_list_file is set but rereco_task_templates is empty; "
                "define at least one task template to expand per run."
            )
        path = resolve_run_list_path(run_list_file, base)
        if not path.is_file():
            raise FileNotFoundError(f"run list file not found: {path}")
        min_run = int(config.get("min_run") or 0)
        max_run = int(config.get("max_run") or 0)
        runs = filter_runs(parse_runs_file(path), min_run, max_run)
        if not runs:
            raise ValueError(
                f"no runs selected from {path} with min_run={min_run} max_run={max_run}"
            )
        tasks = expand_tasks_from_run_list(templates, runs)
        for task in tasks:
            task.setdefault("action", "trirec")
        return tasks

    raw = config.get("rereco_tasks", config.get("rereco_jobs", [])) or []
    tasks = []
    for item in raw:
        task = dict(item)
        task.setdefault("action", "trirec")
        tasks.append(task)
    return tasks
