# ProEQ firmware

Arduino sketch for the ProEQ main board, in `ProEQ/` — the folder name matches
the `.ino` name, so the Arduino IDE opens it directly with no copying around.

Imported from the project's own working copy at
`~/Documents/ArduinoDev/proeq_IS_MCP_V2/` (the newer of `proeq_IS_MCP` and
`proeq_IS_MCP_V2`) and byte-identical to it: `.ino` md5
`42adc70a0ed5a1b0be553c089027e123`.

## Files

| File | Role |
|------|------|
| `ProEQ/ProEQ.ino` | Main sketch: encoder polling, LCD, MIDI CC output |
| `ProEQ/IS_functions.h` / `.cpp` | IS31FL3743 I2C LED driver: init, scaling, LED ring / VU writes |
| `ProEQ/SPI_functions.h` | MCP23S17 SPI driver + encoder/switch mapping tables (header-only) |
| `ProEQ/ACE128map12345678.h` | Bourns ACE-128 gray-code → 0..127 lookup table |
| `ProEQ/ACE_Functions.h` / `.cpp` | ACE helper stubs (`ACE_Functions.cpp` is empty) |

## Building

Open `ProEQ/ProEQ.ino` in the Arduino IDE. Libraries: `LiquidCrystal`, `Wire`
and `SPI` ship with the IDE. Target board per `../docs/wiring-map.txt` is an
Arduino (ATmega328, MIDI over the 16u2 USB-serial).

## State of the code

This is a working prototype, not a finished product. As committed:

- **No MIDI is sent.** `sendMidiCC()` exists but every call to it is commented
  out, so the sketch reads encoders and drives LEDs without ever talking to the
  plug-in.
- `Serial.begin()` opens at `DEBUG_BAUD_RATE` (9600), not at the MIDI rate.
  `MIDI_BAUD_RATE` is declared as 31250 and never used.
- The LCD / LED-ring refresh block keys off a hardcoded `relativeValues[15]` and
  `relativeValues[16]` regardless of which encoder was polled.
- The encoder front end (18 encoders over SPI through the ten MCP23S17, gray
  code mapped to 0..127) and the LED side (two IS31FL3743A, scaling, rings and a
  VU mode) are implemented and were bench-tested.

The part of this sketch worth trusting even while the rest is unfinished is the
wiring map: `SPI_functions.h` states which encoder sits on which expander port
and which switch on which bit. It is cross-checked against
`../docs/wiring-map.txt`, and the two agree.

## Licence

MIT — see [`LICENSE`](LICENSE). The hardware design files elsewhere in this
repository are CERN-OHL-W-2.0; see [`../LICENSES.md`](../LICENSES.md).
