# CHIP-8 Emulator

A basic CHIP-8 interpreter/emulator written in C (C17) using [SDL2](https://www.libsdl.org/).

---

## Prerequisites

- **C Compiler**: Supporting C17 (GCC, Clang, or MSVC)
- **CMake**: Version 3.15 or newer
- **SDL2**: [Simple DirectMedia Layer (SDL2)](https://www.libsdl.org/)

---

## Use

```bash
cmake -B build -S .
cmake --build build

./build/chip-8 "<Game name>.ch8"
```
---

## Controls

The original CHIP-8 keypad layout (0x0 to 0xF) is mapped to standard QWERTY keyboard keys:

```
CHIP-8 Keypad          Keyboard Key
+---+---+---+---+      +---+---+---+---+
| 1 | 2 | 3 | C |      | 1 | 2 | 3 | 4 |
+---+---+---+---+      +---+---+---+---+
| 4 | 5 | 6 | D | ---> | Q | W | E | R |
+---+---+---+---+      +---+---+---+---+
| 7 | 8 | 9 | E |      | A | S | D | F |
+---+---+---+---+      +---+---+---+---+
| A | 0 | B | F |      | Z | X | C | V |
+---+---+---+---+      +---+---+---+---+
```

---

## ROMs & Resources

- **CHIP-8 ROMs Repository**: You can find more games, at [kripod/chip8-roms](https://github.com/kripod/chip8-roms/tree/master).