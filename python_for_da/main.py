"""Launch a project script with its own directory as the working directory."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import Sequence


def parse_args(argv: Sequence[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Run a Python script in the same uv environment. "
            "The script's parent directory becomes its current working directory."
        )
    )
    parser.add_argument("script", type=Path, help="path to the Python script")
    parser.add_argument(
        "script_args",
        nargs=argparse.REMAINDER,
        help="arguments passed unchanged to the target script",
    )
    return parser.parse_args(argv)


def main(argv: Sequence[str] | None = None) -> int:
    args = parse_args(argv)
    script = args.script.expanduser()

    if not script.is_absolute():
        script = Path.cwd() / script

    script = script.resolve()
    if not script.is_file():
        raise SystemExit(f"Ошибка: файл скрипта не найден: {script}")

    completed = subprocess.run(
        [sys.executable, str(script), *args.script_args],
        cwd=script.parent,
        check=False,
    )
    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())
