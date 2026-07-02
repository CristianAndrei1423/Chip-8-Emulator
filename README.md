# Chip-8 Emulator

Chip-8 is a simple, interpreted programming language from the mid-1970s, originally
designed to make it easier to write games for 8-bit microcomputers like the COSMAC VIP.
This project emulates the CHIP-8 virtual machine to run original ROMs and games.

## Features

 - Full chip-8 instruction set
 - Correct `VF` flag timing, including edge cases where `VF` itself is used
 as an operand
 - Different speeds for the CPU execution and the rendering
 - 64x32 display, scaled x10 up for a real window
 - keyboard input mapped to old keypad
 - sprite clipping and origin wrapping matching the original COSMAC VIP

## Requirements

- C compiler (GCC or Clang)
- SDL2 libraries
- make

To install sdl on linux run :
```
sudo apt install libsdl2-dev
```

## Build & Run

```bash
make
./main Games/game_you_want
```

## Controls

The original keypad had to be translated to keys on the keyboard :

```
Arcade       Keyboard
1 2 3 A      1 2 3 4
4 5 6 B  ->  Q W E R
7 8 9 C      A S D F
D 0 E F      Z X C V
```

## Project structure

 
| File          | Purpose                                                          |
|---------------|------------------------------------------------------------------|
| `main.c`      | Window/renderer/audio setup, the main loop, input handling       |
| `chip8.c`     | The CHIP-8 core: memory, registers, fetch/decode/execute         |
| `chip8.h`     | The `Chip8` struct and function declarations                     |

## Acknowledgements
- [Timendus' chip8-test-suite](https://github.com/Timendus/chip8-test-suite)
  for the test ROMs used to validate this emulator
- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
  and [Tobias V. Langhoff's guide](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
  for opcode documentation

Enjoy!