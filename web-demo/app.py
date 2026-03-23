from __future__ import annotations

import os
import re
import subprocess
import tempfile
from pathlib import Path
from typing import Any

from flask import Flask, jsonify, render_template, request

APP_ROOT = Path(__file__).resolve().parent
REPO_ROOT = APP_ROOT.parent
TESTS_DIR = REPO_ROOT / "tests"
SECURE_CLANG = REPO_ROOT / "secure-clang"

app = Flask(__name__)


def _find_line_hints(source_code: str, messages: list[str]) -> list[dict[str, Any]]:
    lines = source_code.splitlines()
    hints: list[dict[str, Any]] = []

    # Map common violation keywords to line-level hints in source.
    keyword_map = {
        "printf": "Sensitive logging API",
        "puts": "Sensitive logging API",
        "fprintf": "Sensitive logging API",
        "perror": "Sensitive logging API",
        "rand": "Weak entropy API",
        "srand": "Weak entropy API",
        "sha1": "Weak crypto API",
        "md5": "Weak crypto API",
        "install": "Install path check",
        "verifysignature": "Signature dominance check",
        "sourcetrusted": "Trusted source check",
        "validatesource": "Trusted source check",
    }

    lower_msgs = "\n".join(messages).lower()

    for idx, line in enumerate(lines, start=1):
        normalized = line.lower()
        for key, label in keyword_map.items():
            if key in normalized and key in lower_msgs:
                hints.append(
                    {
                        "line": idx,
                        "kind": "warning",
                        "label": label,
                        "source": line.strip(),
                    }
                )
                break

    return hints


def _extract_clang_syntax_hints(raw_output: str) -> list[dict[str, Any]]:
    hints: list[dict[str, Any]] = []
    # Example: /tmp/file.c:12:5: error: ...
    pattern = re.compile(r"[^:\n]+:(\d+):(\d+):\s+error:\s+(.*)")
    for match in pattern.finditer(raw_output):
        hints.append(
            {
                "line": int(match.group(1)),
                "column": int(match.group(2)),
                "kind": "error",
                "label": match.group(3).strip(),
            }
        )
    return hints


def run_secure_compile(source_code: str, filename: str) -> dict[str, Any]:
    if not SECURE_CLANG.exists():
        return {
            "ok": False,
            "status": "setup-error",
            "message": "secure-clang wrapper was not found in repository root.",
            "rawOutput": "",
            "violations": [],
            "hints": [],
        }

    safe_name = Path(filename).name
    if not safe_name.endswith(".c"):
        safe_name += ".c"

    with tempfile.TemporaryDirectory(prefix="ota-demo-") as tmp:
        tmp_path = Path(tmp)
        src_path = tmp_path / safe_name
        out_path = tmp_path / "demo.out"
        src_path.write_text(source_code, encoding="utf-8")

        cmd = [str(SECURE_CLANG), str(src_path), "-o", str(out_path)]

        proc = subprocess.run(
            cmd,
            cwd=REPO_ROOT,
            capture_output=True,
            text=True,
            timeout=30,
        )

        raw_output = (proc.stdout or "") + ("\n" if proc.stdout and proc.stderr else "") + (proc.stderr or "")

        violations = [
            line.strip()[2:].strip()
            for line in raw_output.splitlines()
            if line.strip().startswith("-")
        ]

        hints = _find_line_hints(source_code, violations)
        hints.extend(_extract_clang_syntax_hints(raw_output))

        if proc.returncode == 0:
            return {
                "ok": True,
                "status": "passed",
                "message": "Compilation passed. No security violations were reported.",
                "rawOutput": raw_output.strip(),
                "violations": [],
                "hints": hints,
            }

        status = "failed"
        if "setup-error" in raw_output.lower() or "not found" in raw_output.lower():
            status = "setup-error"

        return {
            "ok": False,
            "status": status,
            "message": "Compilation blocked by secure-clang policy checks.",
            "rawOutput": raw_output.strip(),
            "violations": violations,
            "hints": hints,
        }


@app.route("/")
def index() -> str:
    return render_template("index.html")


@app.get("/api/samples")
def list_samples() -> Any:
    samples = []
    for path in sorted(TESTS_DIR.glob("*.c")):
        samples.append(path.name)
    return jsonify({"samples": samples})


@app.get("/api/samples/<name>")
def get_sample(name: str) -> Any:
    sample_path = (TESTS_DIR / Path(name).name).resolve()
    if TESTS_DIR.resolve() not in sample_path.parents and sample_path != TESTS_DIR.resolve():
        return jsonify({"error": "Invalid sample path"}), 400

    if not sample_path.exists() or sample_path.suffix != ".c":
        return jsonify({"error": "Sample not found"}), 404

    return jsonify({"name": sample_path.name, "code": sample_path.read_text(encoding="utf-8")})


@app.post("/api/compile")
def compile_code() -> Any:
    payload = request.get_json(silent=True) or {}
    source_code = payload.get("code", "")
    filename = payload.get("filename", "firmware_demo.c")

    if not source_code.strip():
        return jsonify({"error": "Code is empty."}), 400

    result = run_secure_compile(source_code, filename)
    return jsonify(result)


if __name__ == "__main__":
    # Host 0.0.0.0 for Ubuntu VM browser access.
    app.run(host="0.0.0.0", port=int(os.environ.get("PORT", "5050")), debug=False)
