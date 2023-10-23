# GameboyEmulator
A gameboy (DMG model) emulator.

## Current status
Currently, all CPU instructions, interrupts, hardware timer and basic memory operations are implemented. This theoretically should be enough to run any ROMs that are not using MBC chips, however, most programs require PPU (a WIP at the point of writing his) implementation to fully function.

The project was tested on Windows and Linux using unit tests and test ROMs (blargg's test ROMs were used). Note that version at `dev` branch might not pass all tests (you should use version at `stable` branch instead).

## Dependencies
 - GLAD
 - GLFW
 - Dear ImGui
 - Catch2
## Building
Run cmake at project's root directory. C++20 is required for building the project.
## Testing
Tests' source code is located under src/tests directory. To build tests add `-DBUILD_TESTS=ON` flag when generating build files. Tests can be run with ctest. When running tests' executable directly, make sure to launch it from src/tests/intergation/roms directory as this is where test ROMs are located.
## TODO list
- instr_timing test is currently failing
- Finish PPU implementation
- CPU instructions are displayed in the GUI with delay
