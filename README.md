# GameboyEmulator
A gameboy (DMG model) emulator.
## About

## Current status
Currently, CPU, basic memory operations, interrupts and console I/O are implemented. This theoretically should be enough to run any ROMs that are not using MBC chips. However, most programs require PPU implementation to fully function.

The project was tested on Windows and Linux using unit tests and test ROMs (blargg's test ROMs were used). Note that version at `dev` branch might not pass all tests (you should use version at `stable` branch instead).

## Dependencies
 - GLAD
 - GLFW
 - Dear ImGui
 - Catch2
## Building
Run cmake at project's root directory. Cmake 3.20 was originally used, however, version 3.1 should also work. C++17 is required for building the project.
## Testing
Tests' source code is located under src/tests directory. To build tests add `-DBUILD_TESTS=ON` flag when generating build files.
Currently, the emulator passes all CPU tests.
## TODO list
- Fix timer
- Implement LCD and video chip
- Print debugging info (CPU registers, memory, etc)
- Implement console I/O with curses library
- Keyboard I/O
