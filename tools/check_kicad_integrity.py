#!/usr/bin/env python3
"""Static integrity check for the ProEQ KiCad sources.

Purpose: prove that a rename / move did not break the project, without needing
KiCad installed on the machine doing the rename.

Checks, per KiCad project (.kicad_pro):
  1. SHEET    every (property "Sheetfile" ...) in a .kicad_sch resolves to a file
  2. SYMBOL   every symbol lib_id nickname resolves through sym-lib-table
  3. FOOTPRINT every footprint lib nickname resolves through fp-lib-table
  4. LIBFILE  every library listed in fp/sym-lib-table points at a real path
  5. MODFILE  every footprint referenced through a local .pretty exists as .kicad_mod
  6. MODEL    every 3D model path is reachable: ${KIPRJMOD} relative paths resolve,
              absolute host paths (C:/..., /home/...) are reported as broken
  7. ABSURI   library URIs that are absolute host paths instead of ${KIPRJMOD}

Usage:
  python3 tools/check_kicad_integrity.py           # human report
  python3 tools/check_kicad_integrity.py --json    # machine summary
Exit code is 1 when at least one ERROR is found, else 0.

Known exceptions live in .kicad-integrity-ignore at the repository root, one
'<path fragment>  # reason' per line. An allowlisted finding is reported as
KNOWN instead of ERROR, so the checker can stay green while the reason stays
written down next to the check that produced it.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SKIP_DIRS = {".git", ".history", "__pycache__", "node_modules"}

RE_SHEETFILE = re.compile(r'\(property\s+"Sheetfile"\s+"((?:[^"\\]|\\.)*)"')
RE_LIBID = re.compile(r'\(lib_id\s+"([^"]+)"')
RE_FOOTPRINT = re.compile(r'\(footprint\s+"([^"]+)"')
RE_MODEL = re.compile(r'\(model\s+"((?:[^"\\]|\\.)*)"')
RE_LIB_HEAD = re.compile(r'\(lib\s+\(name\s+"([^"]+)"\)')
RE_URI = re.compile(r'\(uri\s+"((?:[^"\\]|\\.)*)"\)')

# KiCad stock libraries: resolved from the user's global tables, so a nickname
# that is not defined in a project table is only a problem when it is NOT one
# of these. Extend the lists if a design starts using another stock library.
STOCK_FP = {
    "Capacitor_SMD", "Capacitor_THT", "Connector_PinHeader_2.54mm",
    "Connector_PinSocket_2.54mm", "Diode_SMD", "Inductor_SMD", "Module",
    "MountingHole", "Package_SO", "Package_TO_SOT_SMD", "Resistor_SMD",
    "Package_QFP", "Package_DFN_QFN", "Button_Switch_Keyboard",
    "Button_Switch_THT", "TestPoint", "Fiducial", "Connector_USB", "Crystal_SMD",
}
STOCK_SYM = {
    "Device", "power", "Mechanical", "Connector", "Connector_Generic",
    "Interface_Expansion", "Regulator_Switching", "Diode", "Switch",
    "Amplifier_Audio", "Transistor_FET", "74xx", "Memory_EEPROM", "MCU_Microchip_ATmega",
}


def walk_files(root, suffix):
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for name in filenames:
            if name.endswith(suffix):
                yield os.path.join(dirpath, name)


def read(path):
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        return fh.read()


def parse_lib_table(path):
    """Return {nickname: uri} from a fp-lib-table / sym-lib-table.

    Entries are split on their `(lib (name "...")` heads rather than matched
    with a nested regex, which would stop at the first closing paren.
    """
    if not os.path.isfile(path):
        return {}
    text = read(path)
    heads = list(RE_LIB_HEAD.finditer(text))
    out = {}
    for i, head in enumerate(heads):
        end = heads[i + 1].start() if i + 1 < len(heads) else len(text)
        uri = RE_URI.search(text, head.end(), end)
        if uri:
            out[head.group(1)] = uri.group(1)
    return out


def project_lib_table(prj_dir, filename):
    """Return the lib table KiCad actually reads for a project.

    KiCad resolves fp-lib-table / sym-lib-table from the project directory
    only; it never walks up parent directories. Searching upwards would
    attribute the root project's tables to a nested sub-project and invent
    breakage that KiCad would never hit.
    """
    candidate = os.path.join(prj_dir, filename)
    if os.path.isfile(candidate):
        return parse_lib_table(candidate)
    return {}


def resolve_uri(uri, prj_dir):
    """Expand the placeholders a project-local library URI can use."""
    path = uri.replace("${KIPRJMOD}", prj_dir).replace("$(KIPRJMOD)", prj_dir)
    return os.path.normpath(path)


def looks_absolute_host(uri):
    return bool(re.match(r"^[A-Za-z]:[\\/]", uri)) or uri.startswith("/")


def load_allowlist():
    """Read .kicad-integrity-ignore: '<path fragment>  # reason' per line."""
    path = os.path.join(ROOT, ".kicad-integrity-ignore")
    if not os.path.isfile(path):
        return []
    out = []
    for line in read(path).splitlines():
        line = line.split("#", 1)[0].strip()
        if line:
            out.append(line)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    findings = []  # (severity, check, path, detail)

    def add(sev, check, path, detail):
        findings.append((sev, check, os.path.relpath(path, ROOT), detail))

    projects = sorted(walk_files(ROOT, ".kicad_pro"))
    if not projects:
        print("No .kicad_pro found under", ROOT)
        return 1

    # ---- 4 / 7: library tables ------------------------------------------
    tables = {}
    for prj in projects:
        prj_dir = os.path.dirname(prj)
        fp_libs = project_lib_table(prj_dir, "fp-lib-table")
        sym_libs = project_lib_table(prj_dir, "sym-lib-table")
        tables[prj] = (fp_libs, sym_libs)

        for kind, libs in (("fp-lib-table", fp_libs), ("sym-lib-table", sym_libs)):
            for nick, uri in libs.items():
                if looks_absolute_host(uri):
                    add("ERROR", "ABSURI", prj, f'{kind} "{nick}" -> {uri} (absolute host path)')
                    continue
                target = resolve_uri(uri, prj_dir)
                if not os.path.exists(target):
                    add("ERROR", "LIBFILE", prj, f'{kind} "{nick}" -> {uri} (missing)')

    # ---- 1 / 2 / 3 / 5 / 6: per design file ------------------------------
    # Map each schematic to its owning project (nearest .kicad_pro above it).
    sym_name_cache = {}

    def symbol_names(lib_file):
        """Top-level symbol names inside a .kicad_sym, cached per file."""
        if lib_file not in sym_name_cache:
            names = set()
            if os.path.isfile(lib_file):
                for m in re.finditer(r'\(symbol\s+"((?:[^"\\]|\\.)*)"', read(lib_file)):
                    names.add(m.group(1))
            sym_name_cache[lib_file] = names
        return sym_name_cache[lib_file]

    def owner_project(path):
        cur = os.path.dirname(os.path.abspath(path))
        best = None
        for prj in projects:
            pd = os.path.dirname(prj)
            if cur == pd or cur.startswith(pd + os.sep):
                if best is None or len(pd) > len(os.path.dirname(best)):
                    best = prj
        return best

    for sch in sorted(walk_files(ROOT, ".kicad_sch")):
        prj = owner_project(sch)
        fp_libs, sym_libs = tables.get(prj, ({}, {}))
        sch_dir = os.path.dirname(sch)
        text = read(sch)
        for ref in sorted(set(RE_SHEETFILE.findall(text))):
            target = os.path.normpath(os.path.join(sch_dir, ref))
            if not os.path.isfile(target):
                add("ERROR", "SHEET", sch, f"Sheetfile -> {ref} (missing)")
        for lib_id in sorted(set(RE_LIBID.findall(text))):
            if ":" not in lib_id:
                continue
            nick = lib_id.split(":", 1)[0]
            if nick not in sym_libs:
                if nick in STOCK_SYM:
                    continue          # stock library, resolved from the global table
                add("ERROR", "SYMBOL", sch,
                    f'lib_id "{lib_id}" (nickname in no sym-lib-table, not a stock library)')
                continue
            # nickname resolves: check the named symbol is really in that library
            sym = lib_id.split(":", 1)[1]
            if prj is None:
                continue              # archived sheet with no project: cannot evaluate
            lib_file = resolve_uri(sym_libs[nick], os.path.dirname(prj))
            if not lib_file.endswith(".kicad_sym"):
                continue              # not a symbol library, nothing to inspect
            if sym not in symbol_names(lib_file):
                add("ERROR", "SYMNAME", sch,
                    f'lib_id "{lib_id}" (symbol not in {os.path.basename(lib_file)})')

    for pcb in sorted(walk_files(ROOT, ".kicad_pcb")):
        prj = owner_project(pcb)
        fp_libs, _ = tables.get(prj, ({}, {}))
        prj_dir = os.path.dirname(prj) if prj else os.path.dirname(pcb)
        text = read(pcb)
        for fid in sorted(set(RE_FOOTPRINT.findall(text))):
            if ":" not in fid:
                continue
            nick, name = fid.split(":", 1)
            if nick not in fp_libs:
                if nick in STOCK_FP:
                    continue
                add("ERROR", "FOOTPRINT", pcb,
                    f'footprint "{fid}" (nickname in no fp-lib-table, not a stock library)')
                continue
            lib_dir = resolve_uri(fp_libs[nick], prj_dir)
            if os.path.isdir(lib_dir):
                mod = os.path.join(lib_dir, name + ".kicad_mod")
                if not os.path.isfile(mod):
                    add("ERROR", "MODFILE", pcb, f'footprint "{fid}" -> {name}.kicad_mod (missing)')
        for model in sorted(set(RE_MODEL.findall(text))):
            if not model or "_3DMODEL_DIR" in model:
                continue
            if "${KIPRJMOD}" in model or "$(KIPRJMOD)" in model:
                target = resolve_uri(model, prj_dir)
                if not os.path.exists(target):
                    add("ERROR", "MODEL", pcb, f"3D model -> {model} (missing)")
            elif looks_absolute_host(model):
                add("ERROR", "MODEL", pcb, f"3D model -> {model} (absolute host path)")
            elif os.path.exists(os.path.join(prj_dir, model)):
                add("WARN", "MODEL", pcb, f"3D model -> {model} (bare name, resolves only by luck)")
            else:
                add("ERROR", "MODEL", pcb, f"3D model -> {model} (bare name, not found)")

    for mod in sorted(walk_files(ROOT, ".kicad_mod")):
        prj = owner_project(mod) or os.path.join(ROOT, "ProjectEQ.kicad_pro")
        prj_dir = os.path.dirname(prj)
        for model in sorted(set(RE_MODEL.findall(read(mod)))):
            if "_3DMODEL_DIR" in model:
                continue
            if "${KIPRJMOD}" in model or "$(KIPRJMOD)" in model:
                if not os.path.exists(resolve_uri(model, prj_dir)):
                    add("ERROR", "MODEL", mod, f"3D model -> {model} (missing)")
            elif looks_absolute_host(model):
                add("ERROR", "MODEL", mod, f"3D model -> {model} (absolute host path)")

    # ---- report ----------------------------------------------------------
    allow = load_allowlist()
    resolved = []
    for sev, check, path, detail in findings:
        if any(token in detail or token in path for token in allow):
            resolved.append(("KNOWN", check, path, detail))
        else:
            resolved.append((sev, check, path, detail))
    findings = resolved

    order = {"ERROR": 0, "WARN": 1, "KNOWN": 2}
    findings.sort(key=lambda f: (order.get(f[0], 2), f[1], f[2], f[3]))
    counts = {"ERROR": 0, "WARN": 0, "KNOWN": 0}
    for sev, _, _, _ in findings:
        counts[sev] = counts.get(sev, 0) + 1

    if args.json:
        print(json.dumps({"counts": counts,
                          "findings": [dict(zip(("severity", "check", "path", "detail"), f))
                                       for f in findings]}, indent=1))
    else:
        current = None
        for sev, check, path, detail in findings:
            if check != current:
                print(f"\n[{check}]")
                current = check
            print(f"  {sev:5s} {path}\n          {detail}")
        print(f"\n=== {counts.get('ERROR', 0)} error(s), {counts.get('WARN', 0)} warning(s), "
              f"{counts.get('KNOWN', 0)} known exception(s) "
              f"across {len(projects)} project(s) ===")

    return 1 if counts.get("ERROR") else 0


if __name__ == "__main__":
    sys.exit(main())
