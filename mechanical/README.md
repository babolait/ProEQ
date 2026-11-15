# Mechanical sources

CAD sources for the parts of the enclosure that the PCB has to fit. Unlike
`../manufacturing/`, these are editable sources rather than fab outputs.

| File | What it is |
|---|---|
| `proeq-frontplate-flat-v5.FCStd` | FreeCAD source of the flat front plate, revision V5 |
| `proeq-frontplate-flat-v5.step` | STEP export of the same plate |
| `vu-meter.FCStd` | FreeCAD source of the VU meter light guide assembly |
| `blender-notes.txt` | Notes on the Blender render setup used for the panel images |

The `v5` and the original file dates come from the FreeCAD backup file
`freeCad_plate_V5.20260104-144620.FCBak`, which records when V5 was last saved.

Laser-cut profiles for the panel live in `../manufacturing/panel-cut/`.

## Not here

The assembly-level 3D data is large and regenerable, so it is published as
release assets instead: the folded-steel enclosure body, the cut front-plate
solid, the assembly STL/STEP exports (22–34 MB each) and the 41 MB VLP
light-guide STEP. The board still references three of them;

`../.kicad-integrity-ignore` lists exactly which paths are expected to be absent,
with the reason, so the integrity check stays honest instead of silently passing.
