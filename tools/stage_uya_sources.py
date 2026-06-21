#!/usr/bin/env python3
"""Create a compiler-facing Uya source tree for local apps/tests.

The project keeps examples/ and tests/ at the repository root so they do not
belong to the publishable gui/ library. Current Uya package-mode dependency
collection treats those conventional top-level names poorly, so local builds use
ordinary module aliases inside build/. Benchmarks use the same approach because
the compiler treats conventional benchmark directories as non-library roots.
"""

from __future__ import annotations

import argparse
import shutil
from pathlib import Path


def parse_source_dir(root: Path) -> str:
    source_dir = "."
    section = ""
    manifest = root / "uya.toml"
    if not manifest.exists():
        return source_dir

    for raw_line in manifest.read_text(encoding="utf-8").splitlines():
        line = raw_line.split("#", 1)[0].strip()
        if not line:
            continue
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1].strip()
            continue
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip().strip('"')
        if section == "package" and key == "source-dir":
            source_dir = value
        elif section == "layout" and key == "source_dir" and source_dir == ".":
            source_dir = value
    return source_dir


def copy_tree(src: Path, dst: Path) -> None:
    if not src.exists():
        return
    shutil.copytree(
        src,
        dst,
        ignore=shutil.ignore_patterns("__pycache__", ".pytest_cache"),
    )


def copy_source_root(src_root: Path, stage: Path) -> None:
    for path in src_root.iterdir():
        dst = stage / path.name
        if path.is_dir():
            copy_tree(path, dst)
        elif path.is_file():
            shutil.copy2(path, dst)


def rewrite_uya_imports(
    stage: Path,
    examples_alias: str,
    tests_alias: str,
    benchmarks_alias: str,
    source_dir: str,
) -> None:
    replacements = {
        "examples.": f"{examples_alias}.",
        "tests.": f"{tests_alias}.",
        "gui.benchmarks.": f"{benchmarks_alias}.",
    }
    if source_dir not in ("", "."):
        replacements[f"../{source_dir}/"] = "../"
    for path in stage.rglob("*.uya"):
        text = path.read_text(encoding="utf-8")
        rewritten = text
        for old, new in replacements.items():
            rewritten = rewritten.replace(old, new)
        if rewritten != text:
            path.write_text(rewritten, encoding="utf-8")


def write_manifest(stage: Path, examples_alias: str, tests_alias: str) -> None:
    manifest = f"""[package]
name = "gui_uya_stage"
version = "0.0.0"
description = "Generated local build view for gui-uya."

[layout]
source_dir = "."
test_dir = "{tests_alias}"
bench_dir = "benchmarks"
example_dir = "{examples_alias}"
"""
    (stage / "uya.toml").write_text(manifest, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True, type=Path)
    parser.add_argument("--stage", required=True, type=Path)
    parser.add_argument("--examples-alias", default="demo")
    parser.add_argument("--tests-alias", default="suite")
    parser.add_argument("--benchmarks-alias", default="bench")
    args = parser.parse_args()

    root = args.root.resolve()
    stage = args.stage.resolve()
    if stage == root or stage in root.parents:
        raise SystemExit(f"refusing invalid stage path: {stage}")

    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)

    source_dir = parse_source_dir(root)
    source_root = (root / source_dir).resolve()
    if not source_root.exists() or not source_root.is_dir():
        raise SystemExit(f"source_dir does not exist or is not a directory: {source_root}")

    copy_source_root(source_root, stage)
    copy_tree(source_root / "gui" / "benchmarks", stage / args.benchmarks_alias)
    copy_tree(root / "apps", stage / "apps")
    copy_tree(root / "examples", stage / args.examples_alias)
    copy_tree(root / "tests", stage / args.tests_alias)

    rewrite_uya_imports(stage, args.examples_alias, args.tests_alias, args.benchmarks_alias, source_dir)
    write_manifest(stage, args.examples_alias, args.tests_alias)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
