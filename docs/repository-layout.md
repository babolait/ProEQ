# Repository layout, and the mapping from the old one

This repository was reorganised on 2026-11-02. File and folder names are now
English and describe function. No file content was rewritten except for the
references needed to keep the KiCad projects resolving; the electrical design is
byte-identical apart from those paths.

## Why the KiCad project stayed at the repository root

`${KIPRJMOD}` is the directory holding the `.kicad_pro`, and every library and
3D-model path in the design is written relative to it. Keeping the project at the
root means `libraries/…` instead of `../libraries/…` everywhere, and the two
sub-projects only need one level of `..`. Moving the project into `hardware/`
would work but adds indirection to every path in the design for no gain in a
repo that is, for now, a hardware repo.

## Old → new

### Main project

| Old | New |
|---|---|
| `ProjectEQ.kicad_pro` / `.kicad_sch` / `.kicad_pcb` | `ProEQ.kicad_pro` / `.kicad_sch` / `.kicad_pcb` |
| `PRO EQ.kicad_sym` | `libraries/symbols/ProEQ.kicad_sym` |
| `PRO EQ.pretty/` (11 footprints, 2 in use) | `libraries/footprints/ProEQ.pretty/`, only `M10 1P11` kept (as `M10_1P11`) |
| `ProjectEQ_PROTOBOARD/PROEQ.pretty/` (26 footprints, 330 in use) | `libraries/footprints/ProEQ.pretty/` |
| `SYMB/` (per-part symbols, footprints and 3D, mixed) | `libraries/symbols/vendor/`, `libraries/3dmodels/`, rest to `archive/libraries-v1/` |

### Schematic sheets

| Old file | New file | New sheet name |
|---|---|---|
| `Alimentation.kicad_sch` | `schematics/power-supply.kicad_sch` | Power supply |
| `MCU _peripherals.kicad_sch` | `schematics/mcu-peripherals.kicad_sch` | MCU and peripherals |
| `IS31FL3743A_1.kicad_sch` | `schematics/led-driver-1.kicad_sch` | LED driver 1 |
| `IS31FL3743A_2.kicad_sch` | `schematics/led-driver-2.kicad_sch` | LED driver 2 |
| `MCP23017 + EAW.kicad_sch` | `schematics/io-expander-1.kicad_sch` | I/O expander 1 |
| `MCP23017 + EAW_1.kicad_sch` | `schematics/io-expander-2.kicad_sch` | I/O expander 2 |
| `MCP23017 + EAW_2.kicad_sch` | `schematics/io-expander-3.kicad_sch` | I/O expander 3 |
| `MCP23017 + EAW_3.kicad_sch` | `schematics/io-expander-4.kicad_sch` | I/O expander 4 |
| `MCP23017 + EAW_4.kicad_sch` | `schematics/io-expander-5.kicad_sch` | I/O expander 5 |
| `Matias_SW.kicad_sch` | `schematics/keyswitches.kicad_sch` | Keyswitches |
| `MECHANIQUE.kicad_sch` | `schematics/mechanical.kicad_sch` | Mechanical |
| `IS31FL3743A.kicad_sch` | `archive/protoboard/drafts/led-driver-draft.kicad_sch` | unreferenced draft |
| `MCP23017 + EAW_X.kicad_sch` | `archive/protoboard/drafts/io-expander-draft.kicad_sch` | unreferenced draft |

The sheets renamed `MCP23017…` were never MCP23017: the part on both the board
and the BOM is the SPI **MCP23S17**. The new names describe the function
(I/O expander) and drop the wrong part number entirely.

### Footprints renamed

Spaces and `+` were removed; part numbers were kept.

| Old | New |
|---|---|
| `SMLZ24E2N3TT86 + VLP` | `SMLZ24E2N3TT86_VLP` (270 instances) |
| `SW_Matias_QC + 2LED` | `SW_Matias_QC_2LED` (15) |
| `SOT-583-8 Hand solder` | `SOT-583-8_HandSolder` |
| `daughterBoard 363x58` | `DaughterBoard_363x58` |
| `M10 1P11` | `M10_1P11` (the 11-position rotary switch) |
| `midimerleKicad symbol` | `midimerle_logo` |
| `Sans titre` ("untitled") | archived |

### Other trees

| Old | New |
|---|---|
| `Datasheets_ProEQ/` | `datasheets/` — untracked, kept on disk |
| `Prix.txt`, `timeline Proeq.txt` | `private/` — untracked |
| `bom/`, `bom_V2/`, `ProjectEQ*.csv` | `manufacturing/bom/` |
| `SVG for cut/` (DXF, SVG) | `manufacturing/panel-cut/` |
| `SVG for cut/SML_SVG.pretty`, `fp-lib-table` | `archive/libraries-v1/` (orphan library) |
| `cad/` | `mechanical/` |
| `VLP model + ring/` | split: FreeCAD to `mechanical/`, photo to `docs/img/`, duplicate datasheet dropped, `step/` never was in git |
| `Schematics.pdf` | `docs/Schematics.pdf` |
| `hw_list.txt` | `docs/wiring-map.txt` |
| `hw_list_i2c.txt` | `docs/wiring-map-i2c-variant.txt` |
| `Board V1 problems.txt` | `docs/board-v1-issues.txt` (content kept in French) |
| `Daughter_board_V2/` | `daughterboard/` |
| `ProjetEQ daughter board/` | `archive/daughterboard-v1/` |
| `ProjectEQ_PROTOBOARD/` + `…carte fille/` | `archive/protoboard/` + `archive/protoboard/daughterboard/` |
| `firmware/proeq_IS_MCP_V2.ino` | `firmware/ProEQ/ProEQ.ino` (folder must match the sketch name) |
| `LICENSE.txt` (a licence summary) | `LICENSES.md`, rewritten |

## Reference rewrites this required

Renaming files is not enough for KiCad; each of these had to be rewritten in
lockstep, and `tools/check_kicad_integrity.py` verifies all of them:

1. `(property "Sheetfile" …)` in the root schematic — 11 entries.
2. `(property "Sheetname" …)` in the root schematic, and the `sheets` array in
   `ProEQ.kicad_pro` — 11 sheet UUIDs.
3. `meta.filename` in every `.kicad_pro` — KiCad stores its own filename.
4. Library nicknames `PRO EQ` and `PROEQ` → one `ProEQ` symbol nickname and one
   `ProEQ` footprint nickname, across every `lib_id` and `(footprint …)`.
5. Every `(model …)` 3D path in the PCB and in every footprint: `SYMB/<part>/`,
   `VLP model + ring/`, bare filenames and absolute `C:/Users/user/…` /
   `/home/user/…` paths were all rewritten to `${KIPRJMOD}/libraries/3dmodels/`.
6. `fp-lib-table` and `sym-lib-table` at the root, and relative-path tables for
   `daughterboard/` and the two archived projects.
7. The dead `ProjectEQ_PROTOBOARD.pretty` and `Personal:` / `logomidimerle:`
   library entries were dropped; the logo footprint they pointed at was moved
   into `libraries/footprints/ProEQ.pretty/` so it resolves.
