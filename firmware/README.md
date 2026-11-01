# ProEQ firmware

Arduino sketch for the ProEQ main board. Imported 2026-11-01 from the working
copy at `~/Documents/ArduinoDev/proeq_IS_MCP_V2/` (the newer of `proeq_IS_MCP`
and `proeq_IS_MCP_V2`); byte-identical to that source, `.ino` md5
`42adc70a0ed5a1b0be553c089027e123`.

## Files

| File | Role |
|------|------|
| `proeq_IS_MCP_V2.ino` | Main sketch: encoder polling, LCD, MIDI CC output |
| `IS_functions.h` / `.cpp` | IS31FL3743 I2C LED driver: init, scaling, LED ring / VU writes |
| `SPI_functions.h` | MCP23S17 SPI driver + encoder/switch mapping tables (header-only) |
| `ACE128map12345678.h` | Bourns ACE-128 gray-code → 0..127 lookup table |
| `ACE_Functions.h` / `.cpp` | ACE helper stubs (`ACE_Functions.cpp` is empty) |

## Building

The Arduino IDE requires the sketch folder name to match the `.ino` file name.
To build, copy this directory to `proeq_IS_MCP_V2/` (or clone the repo and rename
the `firmware/` directory) and open `proeq_IS_MCP_V2.ino`.

Libraries: `LiquidCrystal`, `Wire` and `SPI` ship with the Arduino IDE. Target
board per `hw_list.txt` is an Arduino (ATmega328/16u2 MIDI over USB-serial).

## Known state (as of the 2026-11-01 import, not changed here)

- `sendMidiCC()` exists but every call to it is commented out, so the sketch as
  committed sends no MIDI.
- The LCD / LED-ring refresh block keys off hardcoded `relativeValues[15]` and
  `relativeValues[16]` regardless of the encoder being polled.
- `MIDI_BAUD_RATE` is declared as 31250 but never used; `setup()` opens
  `Serial` at `DEBUG_BAUD_RATE` (9600).

## Licence

MIT — see `LICENSE` in this directory. The hardware design files elsewhere in
this repository are CERN-OHL-W-2.0 (see the repository root `LICENSE`).
