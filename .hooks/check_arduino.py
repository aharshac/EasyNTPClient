#!/usr/bin/env python3
"""Arduino-specific pre-commit checks from AGENTS.md."""

import re
import subprocess
import sys


def run(cmd):
    return subprocess.run(cmd, capture_output=True, text=True)


def staged_files():
    return run(["git", "diff", "--cached", "--name-only"]).stdout.splitlines()


def staged_content(path):
    r = run(["git", "show", f":{path}"])
    return r.stdout if r.returncode == 0 else None


def staged_diff(path_prefix):
    return run(["git", "diff", "--cached", "--", path_prefix]).stdout


errors = []
staged = staged_files()


# 1. library.properties / library.json version must match
if any(re.match(r"library\.(properties|json)$", f) for f in staged):
    props = staged_content("library.properties") or ""
    json_text = staged_content("library.json") or ""

    m_props = re.search(r"^version=(.+)$", props, re.MULTILINE)
    m_json = re.search(r'"version"\s*:\s*"([^"]+)"', json_text)
    ver_props = m_props.group(1).strip() if m_props else None
    ver_json = m_json.group(1).strip() if m_json else None

    if not ver_props or not ver_json:
        errors.append("Could not parse version from library.properties or library.json")
    elif ver_props != ver_json:
        errors.append(
            f"Version mismatch: library.properties={ver_props} vs library.json={ver_json}"
        )


# 2. No heap allocation added to src/
if any(f.startswith("src/") for f in staged):
    heap_re = re.compile(r"\b(new |malloc\(|calloc\(|realloc\()")
    for line in staged_diff("src/").splitlines():
        if re.match(r"^\+[^+]", line) and heap_re.search(line):
            errors.append(f"Heap allocation in src/ (not allowed): {line[1:].strip()}")
            break


# 3. No Serial debug output added to src/
if any(f.startswith("src/") for f in staged):
    serial_re = re.compile(r"\bSerial\.(print|println|write|begin)\b")
    for line in staged_diff("src/").splitlines():
        if re.match(r"^\+[^+]", line) and serial_re.search(line):
            errors.append(f"Serial output in src/ (not allowed): {line[1:].strip()}")
            break


# 4. NTP epoch constant must remain in EasyNTPClient.cpp
if "src/EasyNTPClient.cpp" in staged:
    content = staged_content("src/EasyNTPClient.cpp") or ""
    if "2208988800" not in content:
        errors.append(
            "NTP epoch constant (2208988800UL) removed from src/EasyNTPClient.cpp"
        )


# 5. Three-constructor public API must be preserved in header
if "src/EasyNTPClient.h" in staged:
    content = staged_content("src/EasyNTPClient.h") or ""
    count = len(re.findall(r"EasyNTPClient\s*\(", content))
    if count < 3:
        errors.append(
            f"Constructor API broken: found {count} EasyNTPClient(...) declaration(s) in header, need ≥3"
        )


if errors:
    print("Arduino library invariant check(s) failed:", file=sys.stderr)
    for e in errors:
        print(f"  - {e}", file=sys.stderr)
    sys.exit(1)

sys.exit(0)
