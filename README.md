# CHIP-8 Emulator

A CHIP-8 emulator written in C using [SDL3](https://libsdl.org) for graphics and input.

CHIP-8 is a tiny virtual machine from the 1970s, originally used to run simple games on hobbyist computers. This project emulates its CPU, memory, timers, 64x32 monochrome display and 16-key keypad, so you can run classic CHIP-8 ROMs on a modern machine.

## Download

Pre-built Windows binaries are available on the [Releases page](../../releases/latest).

1. Download `chip8-emulator-windows-x64.zip` from the latest release.
2. Extract it to any folder. It contains `chip8.exe`, `SDL3.dll` and this README. Keep the two files together.
3. Get a CHIP-8 ROM (a `.ch8` file). This repository does not include game ROMs. Free test ROMs are available from the [Timendus test suite](https://github.com/Timendus/chip8-test-suite).
4. Open Command Prompt or PowerShell in the extracted folder and run:

```
chip8.exe 10 2 "path\to\rom.ch8"
```

Tip: in Command Prompt you can drag a ROM file onto the window to paste its path.

## Features

- All standard CHIP-8 opcodes, dispatched through function-pointer tables instead of one giant `switch`
- 64x32 display, scaled to any window size with nearest-neighbour filtering (crisp pixels)
- Adjustable emulation speed from the command line
- Keyboard mapping for the 16-key hex keypad
- Built-in font sprites
- Single source file, no dependencies besides SDL3

## Usage

```
chip8 <Scale> <Delay> <ROM>
```

| Argument | Meaning |
|---|---|
| `Scale` | Window size multiplier. Each CHIP-8 pixel becomes a `Scale` x `Scale` block (`10` gives a 640x320 window). |
| `Delay` | Minimum milliseconds between CPU cycles. Lower is faster, higher is slower. |
| `ROM` | Path to a `.ch8` ROM file. Wrap the path in quotes if it contains spaces. |

Example:

```
chip8.exe 10 2 roms\2-ibm-logo.ch8
```

If a game runs too fast or too slow, adjust `Delay` (values between 1 and 5 are a good starting range).

## Controls

The original CHIP-8 keypad is mapped onto the left side of a QWERTY keyboard:

```
CHIP-8 keypad        Keyboard
+---+---+---+---+    +---+---+---+---+
| 1 | 2 | 3 | C |    | 1 | 2 | 3 | 4 |
| 4 | 5 | 6 | D |    | Q | W | E | R |
| 7 | 8 | 9 | E |    | A | S | D | F |
| A | 0 | B | F |    | Z | X | C | V |
+---+---+---+---+    +---+---+---+---+
```

Press **Esc** or close the window to quit.

## Building from source

### Requirements

- A C compiler (GCC, Clang or MSVC)
- SDL3 development libraries

### Windows (MSYS2, UCRT64 shell)

Install the toolchain and SDL3:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-SDL3 mingw-w64-ucrt-x86_64-pkgconf
```

Development build (with debug info and warnings):

```bash
gcc -Wall -Wextra -g CHIP8.c -o chip8 $(pkg-config --cflags --libs sdl3)
```

Optimized release build:

```bash
gcc -O2 -s -Wall -Wextra CHIP8.c -o chip8 $(pkg-config --cflags --libs sdl3)
```

If you run the `.exe` outside the MSYS2 shell, copy `SDL3.dll` from `/ucrt64/bin` next to it.

### Linux

Install SDL3 from your package manager (for example `libsdl3-dev`), or build it from source. Then:

```bash
gcc -O2 -Wall -Wextra CHIP8.c -o chip8 $(pkg-config --cflags --libs sdl3)
```

### macOS

```bash
brew install sdl3
gcc -O2 -Wall -Wextra CHIP8.c -o chip8 $(pkg-config --cflags --libs sdl3)
```

## How it works

The emulator is built around a single `CHIP8` struct that holds the whole machine:

| Field | Purpose |
|---|---|
| `reg[16]` | General-purpose registers V0 to VF (VF is the flag register) |
| `mem[4096]` | 4 KB of RAM. Programs load at `0x200`, font sprites at `0x50` |
| `pc`, `index`, `sp`, `stack[16]` | Program counter, index register, stack pointer and call stack |
| `delay_timer`, `sound_timer` | The two countdown timers |
| `keypad[16]` | Current key states, written by the input handler |
| `video_gfx[64*32]` | Display pixels, uploaded to an SDL texture each frame |

Each call to `Cycle()` performs one fetch-decode-execute step:

1. **Fetch** two bytes from memory at `pc` and combine them into a 16-bit opcode.
2. **Advance** `pc` by 2.
3. **Decode** by using the top nibble of the opcode as an index into a table of function pointers. Opcode groups that share a top nibble (`0x0`, `0x8`, `0xE`, `0xF`) use a second-level table.
4. **Execute** the handler function.

The main loop polls input every iteration and runs a CPU cycle only when `Delay` milliseconds have passed since the last one.

## Project layout

```
CHIP8.c     emulator source (CPU, opcodes, SDL platform layer, main loop)
README.md   this file
```

## Testing

A good way to check the emulator is with Timendus's CHIP-8 test suite:

- https://github.com/Timendus/chip8-test-suite

Suggested order: `1-chip8-logo`, `2-ibm-logo`, `3-corax+`, `4-flags`, `5-quirks`. These cover display, arithmetic, flag behaviour and the common interpreter quirks.

## Troubleshooting

| Problem | Fix |
|---|---|
| `SDL3.dll was not found` | Keep `SDL3.dll` in the same folder as `chip8.exe`. |
| Usage message prints even though you gave three arguments | The ROM path probably has a space in it. Wrap it in quotes: `"My Game.ch8"`. |
| `Failed to get data from rom` | Check the path and that the file exists. Make sure the executable you run is the one you just built. |
| Window opens and closes immediately | Run it from a terminal (not by double-clicking) so you can read the error message. |
| `SDL3/SDL.h: No such file or directory` (building) | SDL3 is not installed, or you are in the wrong shell (use UCRT64 on MSYS2). |
| Game runs too fast or slow | Change the `Delay` argument. |

## Resources

- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [SDL3 documentation](https://wiki.libsdl.org/SDL3/FrontPage)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

SDL3 is distributed under the zlib license, which permits redistributing `SDL3.dll` with your release.
