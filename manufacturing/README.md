# Manufacturing data

What you send to a fab, plus the BOM revisions it was built from. Generated
order packages (Gerbers, CPL, position files, IPC netlist) are not committed —
regenerate them with the fabrication toolkit plugin
(`../fabrication-toolkit-options.json`).

## bom/

| File | What it is |
|---|---|
| `proeq-bom.csv` | BOM export, `Id,Designator,Footprint,Quantity,Designation,Supplier and ref` layout |
| `proeq-bom-v2.csv` | Same layout, later export (40 rows vs 36) |
| `proeq-bom-v3.csv` | Different layout: `Reference,Value,Footprint,Description,Qty,Digikey REF` |
| `ibom.html` | Interactive BOM (boardview-style, opens in a browser) |
| `ibom-v2.html` | Second interactive BOM |

The three CSVs are not interchangeable — two layouts, three exports. The
revision each one matches is **not recorded inside the files**, so treat the
numbering as export order, not as a board revision. The two interactive BOMs came
from separate folders and carry no revision stamp either. If you know which
export matches which board, rename them; do not guess.

## panel-cut/

Laser-cut profile for the front plate, exported from the panel outline:

- `proeq-panel-cut.dxf` — CAD interchange format for the cutter
- `proeq-panel-cut.svg` — same profile as vector art

Both describe the same outline; the SVG is the one to open in a browser or
Inkscape, the DXF is the one to hand to the cutter.

## Where the rest lives

- Front plate and VU mechanical sources: `../mechanical/`
- Heavy 3D exports (assembly STL/STEP, enclosure body, VLP light guide):
  GitHub Release assets, listed in `../.kicad-integrity-ignore`
