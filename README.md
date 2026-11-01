# ProEQ — EQ plug-in MIDI controller (hardware)

ProEQ is the hardware half of an EQ plug-in controller: a front panel of 18
absolute encoders, 15 mechanical keyswitches, 2 eleven-position rotary switches
and 270 LEDs around a 16×2 LCD, driven by its own ATmega board over MIDI. This
repository holds the KiCad sources, the BOM/assembly data, the firmware sketch
and the small CAD deliverables. Fabrication outputs and heavy 3D data are
distributed as GitHub Release assets, not as git objects (see "Not in this
repository").

The board went through a fabricated V1 (its fault list is in
[`Board V1 problems.txt`](Board%20V1%20problems.txt), commit-tracked verbatim in
French), a protoboard bring-up stage, and a re-spin that is the current 4-layer
main board. As of 2026-11-01 the re-spin is designed and priced but, per the
punch list below, still unfinished in places.

*Hardware licence: CERN-OHL-W-2.0 ([`LICENSE`](LICENSE)). Firmware licence: MIT
([`firmware/LICENSE`](firmware/LICENSE)).*

## At a glance

Source of the counts: [`hw_list.txt`](hw_list.txt) and the firmware mapping
tables in [`firmware/SPI_functions.h`](firmware/SPI_functions.h).

| Block | Part | Qty | Bus |
|-------|------|-----|-----|
| MCU | Arduino / ATmega328 + USB-serial | 1 | — |
| LED driver | IS31FL3743A (36×11 matrix, PWM dimming) | 2 | I2C, 0x20 and 0x23 |
| LEDs | 270 total (encoder value rings + switch backlight + VU) | 270 | via the 2 drivers |
| I/O expander | MCP23S17 (SPI variant of MCP23017) | 10 | SPI, CS on D2/D3, A2..A0 per chip |
| Encoders | Bourns ACE-128 absolute (8-bit gray code) | 18 | through the expanders |
| Keyswitches | Matias Quiet Click + per-switch LED | 15 | through the expanders |
| Rotary switch | 11-pole 1-throw + resistor ladder | 2 | analog A0 / A1 |
| Display | 16×2 character LCD (HD44780, parallel) | 1 | 4-bit parallel |

MCP23S17 addressing (`hw_list.txt`): U2–U9 on `CS_0` with A2A1A0 = 000…111,
U10 on `CS_1` = 001, U11 on `CS_1` = 000.

Encoder → expander port map and switch → expander bit map are in
`hw_list.txt`, and the same 18-entry encoder map / 15-entry switch map is
hardcoded in `firmware/SPI_functions.h` (`encoderMCP[]`, `encoderReg[]`,
`switchMCP[]`, `switchBit[]`). The two agree.

## Schematic and board

- Root sheet: title `Schéma général`, revision `3`, date `2024-08-18`, company
  `ProEQ : PCB 1`. 12 printed pages (`Schematics.pdf`): the root plus 11
  sub-sheets — `Alimentation`, `MCU _peripherals`, `IS31FL3743A_1`,
  `IS31FL3743A_2`, `MCP23017 + EAW` (and `_1`.. `_4`), `Matias_SW`,
  `MECHANIQUE`.
- Two schematic files sit in the tree but are not referenced by the root
  sheet: `IS31FL3743A.kicad_sch` and `MCP23017 + EAW_X.kicad_sch`. They are
  kept as-is (unreferenced drafts/alternates). Sheet names say `MCP23017`, the
  part actually used and documented is the SPI `MCP23S17`.
- Main board: 4 copper layers (F.Cu, In1 = "Power", In2 = "Gnd", B.Cu),
  1.6 mm, A2 sheet, KiCad 9 format (`version 20241229`).

## Board list

Eight `.kicad_pro` files exist in the working tree. Only the first five are
tracked; the last three are excluded on purpose (see `.gitignore`).

| Project file | What it is | Tracked |
|---|---|---|
| `ProjectEQ.kicad_pro` (root) | Current main board, rev 3 schematics, 19.5 MB PCB, 4 layers | yes |
| `ProjectEQ_PROTOBOARD/ProjectEQ_PROTOBOARD.kicad_pro` | Protoboard / bring-up board for the same peripheral blocks, 1.1 MB PCB | yes |
| `ProjectEQ_PROTOBOARD/ProjectEQ_PROTOBOARD_carte fille/…kicad_pro` | Daughter card for the protoboard, 0.83 MB PCB | yes |
| `Daughter_board_V2/ProEQ daughter.kicad_pro` | Daughter board V2, with a 0.1 MB STEP export | yes |
| `ProjetEQ daughter board/ProEQ daughter/ProEQ daughter.kicad_pro` | First daughter-board tree; its `.kicad_sch` is a 105-byte stub, only the STEP/WRL 3D exports and zipped backups are real | yes (stub) |
| `ProjectEQ_test rearrangeled.kicad_pro` | Layout experiment, 15.4 MB PCB | no — `*rearrangeled*` |
| `ProjectEQ_backup.kicad_pro` | Snapshot of an earlier main-board layout, 8.5 MB PCB | no — `ProjectEQ_backup.*` |
| `Gerber V1 old/old_v1.kicad_pro` | The V1 board as sent to fabrication, 14.8 MB PCB | no — whole `Gerber V1 old/` tree |

**Which PCB is current — stated explicitly, not guessed:** `ProjectEQ.kicad_pcb`
at the repo root is the one the current `ProjectEQ.kicad_pro` drives, and it is
the only part of the source tree whose files carry a 2026-09-03 timestamp (every
other file shows 2026-08-29). `SVG for cut/ProjectEQ.kicad_pcb` is the same
board exported for laser cutting — 19,501,963 bytes versus 19,545,945 at the
root, i.e. not identical — and is ignored as a duplicate. Because file mtimes
here are copy dates, no ordering between the root PCB, the "rearrangeled"
experiment and `ProjectEQ_backup` can be proven from the tree; the root is
called current on the strength of its `.kicad_pro` and its newer project
metadata only.

## Hardware stack

- **IS31FL3743A** ×2 — 36×11 cross-point LED driver with 8-bit PWM per dot, I2C
  at `0x20` (`IS_1`) and `0x23` (`IS_2`), plus a scaling register used to cap
  brightness (`IS_functions.h`).
- **MCP23S17** ×10 — SPI port expanders, 16 inputs each, all configured as
  inputs with pull-ups disabled and interrupts off (`IOCON 0b01111000`,
  `IODIR 0xFF`).
- **Matias Quiet Click** ×15 — mechanical keyswitches, one MCP23S17 port bit
  each (bit order deliberately not sequential: see `switchBit[]`), each with
  backlight LEDs driven from the IS31FL3743A matrix.
- **VLP-450-R light guides** ×270 — the LED-to-frontplate optics; 3D models are
  in `VLP model + ring/` (heavy `.blend`/`.step`, release assets).
- **Bourns ACE-128 absolute encoders** ×18 — 8-bit gray code per revolution,
  decoded through `ACE128map12345678.h`, then treated as 0..127 single-turn or
  scaled to 0..16383 multi-turn in the sketch.
- **EAW rotary switches** ×2 — 11 positions read as a resistor ladder on A0/A1.

## Project state

### Working

- V1 boards were fabricated and assembled: `production/ProjectEQ.zip`,
  BOM/positions/designators exports and the JLC package in `jlcpcb/`, with
  fabrication backup archives dated 2025-01-25.
- Encoder front-end is implemented and bench-tested: 18 encoders read over SPI
  through the ten expanders, gray code mapped to 0..127.
- LED side is implemented: two IS31FL3743A initialised, scaling set, encoder
  rings and a VU mode written over I2C.
- Nearly the whole V1 fault list is closed. In `Board V1 problems.txt` these
  items are marked ✅: Matias holes 15.5×15.5 instead of 15.5×12.8 (board to be
  re-ordered at the right size); daughter-board length misalignment (board to be
  redesigned); swapped C16/R12 labels; C5/C1 confusion in the supply; symbol ↔
  footprint mismatch ("Schéma plus en lien avec le PCB"); Matias cathode
  silkscreen on the wrong side; missing LCD header; external-5V-then-USB
  sequencing (decision: keep "5V out only" for V2); `.25` routing; the SPI light
  idea; and both LCD mounting-screw moves (x=11→21 and x=136→129 at y≈53.5).

### Unfinished, quoted from the project's own notes

From `Board V1 problems.txt`, still open (no ✅):

- `- 0 implantation technique case` → "Peu-être pas un soucis, sinon, carte à
  rerouter" (no case-mounting features on the board).
- `- 3D print a revoir zone daughter, a commander et tester le press fit` —
  daughter-board area of the 3D print still to be reworked, ordered and
  press-fitted.
- `-pcba a JLC` / "est-ce possible de commander les leds et faire du placement
  pour 1 board seuleument" — open question about ordering LEDs and assembly for
  a single board.
- `- Dimensions LCD reverse et lcd pliage changés` — reversed LCD dimensions and
  the changed LCD fold.

From `timeline Proeq.txt` (complete list, all still TODOs):

1. Verify the press-fit size at JLCPCB 3D printing (`jlc3dp`).
2. Test the LM66200 (supply/load switching part named in the timeline).
3. Add it on the PCB.
4. Order a ring and a VU from `jlc3dp`.
5. Test one MCP and one IS at the same time.
6. Order an AVR ISP.
7. Test the AVR chip and the `midikilik` bootloader.
8. Test the firmware with a Fruity (FL Studio) script.
9. Pre-order PCBA at JLC (500 LEDs, IS, etc.).
10. Order PCBA for 2 boards, then 3 boards.

Firmware caveats, from reading `firmware/proeq_IS_MCP_V2.ino`:

- `sendMidiCC()` is implemented but every call is commented out — as committed
  the sketch emits no MIDI.
- The LCD/LED-ring refresh block tests `relativeValues[15]` and
  `relativeValues[16]` regardless of the encoder index in the loop, so only two
  encoders update the display and rings.
- `MIDI_BAUD_RATE = 31250` is declared but never used; `setup()` opens `Serial`
  at `DEBUG_BAUD_RATE` (9600).

**Unresolved discrepancy — LCD pinout.** `hw_list.txt` assigns D4–D7 to the LCD
data lines, D8 = `LCD_E`, D9 = `LCD_RS`, D10 = `LCD_R/W`. The sketch builds
`LiquidCrystal lcd(10, 9, 8, 7, 6, 5, 4)`, i.e. RS = 10, E = 9, D4–D7 = 8, 7, 6,
5, R/W = 4. Both use pins 4–10 but with different roles. Neither document can be
called correct from the repository alone; check the PCB netlist before wiring.

### Cost notes

`Prix.txt`, verbatim: `260 avec enig (+19) pr 2 board PCBA (only top
components)`, remaining `-eaw = 200€`, `-bivar = 100€`. These are 2024–2025
quote figures, not current prices.

## Firmware

`firmware/` is the `proeq_IS_MCP_V2` Arduino sketch (the newer of
`proeq_IS_MCP` / `proeq_IS_MCP_V2`), copied byte-identical on 2026-11-01. See
[`firmware/README.md`](firmware/README.md) for the file roles, the Arduino
folder-name requirement and the known-state list. MIT licensed
([`firmware/LICENSE`](firmware/LICENSE)).

## CAD deliverables

| File | Size | Source |
|---|---|---|
| `cad/proeq_frontplate_flat_v5_2026-01-04.step` | 6.8 MB | `~/Documents/Plate/freeCad_plate_V5-Cut.step` |
| `cad/proeq_frontplate_flat_v5_2026-01-04.FCStd` | 3.4 MB | `~/Documents/Plate/Old/freeCad_plate_V5.FCStd` |

The date in the names comes from the FreeCAD backup file
`freeCad_plate_V5.20260104-144620.FCBak`, which records when V5 was last saved;
it is not the file mtime. V5 is the last flat front-plate revision present;
`plate_V2-Cut.step` and the V3/V4 `-Cut.stl` files are older and larger and were
not copied.

Skipped for size (over the 20 MB per-file limit): the V4/V5/V6 front-plate STL
exports (22–34 MB each), `Unnamed-VLPVu003.step` (41 MB), and the `.blend`
models. They stay as release assets.

## Opening in KiCad

Requires **KiCad 9** — verified with `kicad-cli` 9.0.2 on the machine this repo
was assembled on. The sources are KiCad 9 format (PCB `version 20241229`,
schematic `version 20250114`); KiCad 8 will refuse to open them, and KiCad 9 will
offer to save an older file up to 9 format.

1. Open `ProjectEQ.kicad_pro`.
2. The project tables are relative to the project, so no library setup is
   needed: `fp-lib-table` and `sym-lib-table` point at `${KIPRJMOD}/PRO EQ.pretty`,
   `${KIPRJMOD}/ProjectEQ_PROTOBOARD/PROEQ.pretty` and
   `${KIPRJMOD}/PRO EQ.kicad_sym`, all committed.
3. `production/` and `jlcpcb/` are not in the repo, so the "fabrication outputs"
   plugin (`fabrication-toolkit-options.json`: `AUTO TRANSLATE` and `AUTO FILL`
   on, `EXCLUDE DNP` off) will regenerate them from scratch.

## Not in this repository, and why

Excluded to keep git light; these become GitHub Release assets on the first
release. Full rules are in [`.gitignore`](.gitignore).

| Excluded | Where it lives | Why |
|---|---|---|
| `Gerber/`, `Gerber V1 old/`, `PLOTS/` | working tree | Generated plots; the V1 tree is superseded |
| `jlcpcb/`, `production/` | working tree | Order packages (Gerber zip, BOM/CPL, IPC netlist) |
| `ProjectEQ.step` (37 MB), `ProjectEQ_V2.pcb3d` (14 MB), `Board_render.blend` (35 MB) | working tree | Heavy 3D; regenerable from the PCB |
| `VLP model + ring/*.blend`, `*.stl`, `step/*.step` (up to 31 MB) | working tree | Heavy CAD |
| `ProjectEQ_backup.kicad_pcb` (8.5 MB) and the `*rearrangeled*` layout | working tree | Duplicates of the same design |
| all `*-backups/`, `_autosave-*`, `*.zip`, `*.bak` | working tree | KiCad autosave/backup noise |
| `fp-info-cache`, `*.kicad_prl` | working tree | Regenerated cache / per-machine UI state |
| `board installation timeline` | working tree | Personal diary, no technical content — kept on disk, not published |

Kept on purpose: `Datasheets_ProEQ/` (vendor PDFs, 11 MB — the 10 MB
`Datasheets_ProEQ.zip` duplicate is ignored), `bom/ibom.html` and
`bom_V2/ibom.html` (interactive BOMs, 0.85 MB each), the
`ProjectEQ.csv` / `_V2` / `_V3` BOM revisions, `Schematics.pdf`, `SYMB/`
(symbol/3D assets for the custom parts) and `SVG for cut/` (DXF + SVG cut
profiles).

## Provenance

This repository was assembled in place on **2026-11-01** from a **618 MB**
working tree at `~/Documents/ProEQ` that mixed sources, autosaves, backups,
Gerbers, datasheets and 3D data. No existing file was deleted, moved or renamed
to create it; the only pre-existing files rewritten were `README.md` (was the
Nerd Fonts release readme) and `LICENSE.txt` (was the SIL OFL text of the
Recursive font) — both were copied to `/tmp/proeq-font-debris/` first.

**File mtimes in this repository are copy dates, not edit dates.** Almost
everything carries 2026-08-29 19:55–19:56; three entries (`ProjectEQ.kicad_pro`,
`fp-info-cache`, `ProjectEQ-backups/`) carry 2026-09-03 21:07. Do not use mtime
to rank revisions — ignore lists and content are the only reliable ordering.
Scan sheet names to `kicad_pro` revisions instead.
