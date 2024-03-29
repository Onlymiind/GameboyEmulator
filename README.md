# GameboyEmulator
A Game Boy (DMG model) emulator.

## Current status

### Implemented:
- CPU instructions
- Hardware timer
- Basic memory emulation (some memory-mapped IO registers are implemented as normal RAM)
- Interrupts
- Basic MBC1 chip support

### Partially implemented:
- PPU (implementation is a work in progress: mostly works, but is highly inaccurate, sprites are yet to be tested)

### Features:
- Memory view
- Instruction breakpoints
- Memory breakpoints
- Instruction and frame stepping
- Instruction and CPU registers logging
- Emulation fast-forwarding
- Basic diassembler
    * Instructions are displayed only if they are executed at least once
    * Dynamically changing code (i.e. if code at the same RAM address and bank is modified after first execution) might lead to artifatcs if legths of dynamically changing instructions do not match.

The project was tested on Windows and Linux using unit tests and test ROMs (blargg's test ROMs were used). Note that version at `dev` branch might not pass all tests (you should use version at `stable` branch instead).

## Dependencies
 - GLAD
 - GLFW
 - Dear ImGui
 - Catch2
## Building
Run cmake at project's root directory. C++20 is required for building the project.
## Testing
Tests' source code is located under src/tests directory. To build tests add `-DBUILD_TESTS=ON` flag when generating build files. Tests can be run with ctest.

Note that for tests to pass you need to place blargg's test roms in the `src/tests/integration/roms/blargg_test_roms` directory if running tests with ctest (or just `<path-to-tests'-executable>/blargg_test_roms` if you are running the tests' executable directly). Check `src/tests/integration/integration_test.cpp` for the list of needed roms.

## Usage notes

- Breakpoints are removed by pressing backspace while hovering over them.
- To see the CPU status during some instruction, click on the instruction.
- Currently only 10 most recent instructions can be logged.
- To stop/resume execution press spacebar.
- Press F11 while execution is stopped to advance a single instruction, press F12 to advance a frame.
- Some values of PPU's memory-mapped registers can appear static in memory view, although they shouldn't be (e.g. current scanline). This happens because during normal execution memory view is always drawn at the specific cycle of emulator's frame.

## Tests status

All blargg's tests for CPU instructions as well as instr_timing.gb and memory access timing tests are passing. See src/tests/integration/roms/blargg_test_roms for list of tests. Work is being done for passing some of the mooneye test suite roms.

## Screenshots

![Emulator view](./emulator_view.png)


![Debugger](./debugger.png)


![Memory view](./memory_view.png)

## TODO list
- Properly test PPU
- Make PPU emulation more accurate
- Sprites are not displayed
- Emulator completely fails mooneye tests for MBC1 chip, although it works fine with some banked blargg test roms.
