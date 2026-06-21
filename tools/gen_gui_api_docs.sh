#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
out_file="${1:-"$repo_root/docs/gui_uya_api_reference.md"}"

python3 - "$repo_root" "$out_file" <<'PY'
from pathlib import Path
import re
import sys

repo_root = Path(sys.argv[1])
out_file = Path(sys.argv[2])

decl_re = re.compile(r'^export (mc|const|enum|struct|interface|fn|@async_fn fn)')

lines_out = [
    "# UyaGUI API Reference",
    "",
    "> 该文件由 `tools/gen_gui_api_docs.sh` 从 `src/gui/**/*.uya`、`apps/**/*.uya`、`examples/**/*.uya`、`tests/**/*.uya` 自动生成。",
    "> 内容聚焦公开符号索引，详细设计说明请结合 `docs/gui_uya_*.md` 系列文档阅读。",
    "",
]

source_roots = [
    repo_root / "apps",
    repo_root / "examples",
    repo_root / "src" / "gui",
    repo_root / "tests",
]

for file in sorted(path for root in source_roots for path in root.rglob("*.uya")):
    source_lines = file.read_text(encoding="utf-8").splitlines()
    entries: list[tuple[int, str, list[str]]] = []

    for idx, raw_line in enumerate(source_lines):
        if not decl_re.match(raw_line):
            continue

        docs: list[str] = []
        cursor = idx - 1
        while cursor >= 0:
            prev = source_lines[cursor].strip()
            if prev.startswith("///"):
                docs.append(prev[3:].strip())
                cursor -= 1
                continue
            if prev == "":
                cursor -= 1
                continue
            break
        docs.reverse()

        decl = re.sub(r"\s+", " ", raw_line.strip()).rstrip()
        entries.append((idx + 1, decl, docs))

    if not entries:
        continue

    rel_path = file.relative_to(repo_root).as_posix()
    lines_out.append(f"## `{rel_path}`")
    lines_out.append("")

    for line_no, decl, docs in entries:
        lines_out.append(f"- L{line_no}: `{decl}`")
        if docs:
            lines_out.append(f"  说明: {' '.join(docs)}")
    lines_out.append("")

out_file.write_text("\n".join(lines_out), encoding="utf-8")
PY
