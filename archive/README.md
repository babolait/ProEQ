# Archive

Superseded design data, kept because it explains how the current board got here.
Nothing in this folder is part of the current design, and nothing outside it
references it.

| Path | What it is | Why it is not current |
|---|---|---|
| `protoboard/` | Bring-up board that proved the LED ring dimensions and the IS31 LED matrix, plus its daughter card and two draft sheets | Superseded by the main board, which now carries the same blocks |
| `daughterboard-v1/` | First daughter-board tree | Only a 105-byte stub schematic plus STEP/WRL exports; V2 in `daughterboard/` is the real board |
| `libraries-v1/` | Footprint and symbol libraries from before the shared-library extraction | Superseded, with one exception below |

## The one thing worth knowing about `libraries-v1/`

The current main board used to pull **330 of its 436 footprints** out of the
protoboard project folder. That is why the shared library now lives in
`../libraries/` and this folder keeps only what nothing uses any more.

`libraries-v1/footprints/` holds the superseded `PRO EQ.pretty` geometry. One
footprint in it was still in use — the `M10 1P11` rotary-switch footprint — and
that one was moved into `../libraries/footprints/ProEQ.pretty/` as `M10_1P11`
rather than archived.

`libraries-v1/legacy/` holds the per-part KiCad 5 `.lib` / `.mod` / `.dcm`
sources and duplicate `.kicad_mod` copies that came out of the old `SYMB/`
graveyard folder. The KiCad 9 `.kicad_sym` versions of the same parts are in
`../libraries/symbols/vendor/`.

## Who owns the symbols in `protoboard/PROEQ.kicad_sym`

That file holds `EXB-2HV472JV` and `SMLZ24E2N3TT86` symbols that exist nowhere
else. The archived protoboard schematic references them through its own `PROEQ`
library nickname, which its `sym-lib-table` points at the local file. Do not
rename those to the shared `ProEQ` nickname without moving the symbols too —
`tools/check_kicad_integrity.py` will catch it if you do.
