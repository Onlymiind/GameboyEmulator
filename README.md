# GameboyEmulator
A gameboy (DMG model) emulator.
## About

## Current status
Currently, CPU, basic memory operations and console I/O are implemented. This theoretically should be enough to run any ROMs that are not using MBC chips. However, most programs require LCD to fully function.

The project was tested on Windows and Linux using unit tests and test ROMs (blargg test ROMs were used). Note that version at `main` branch might not pass all tests (you should use version at `stable` branch instead).
## Commands
- `-help` - show list of commands
- `-romdir <path>` - set directory in which ROMs will be looked for.
- `-run <name>` - run a ROM with the specified name. Currently, using path to ROM instead of just name is not supported.  
Use `-romdir` command to set the path.
- `-ls` - list all ROM files in directory set by `-romdir` command.
- `-config` - show current ROM directory
- `-quit` - quit the emulator
## Dependencies
This project has no dependencies.
## Building
Run cmake at project's root directory. Cmake 3.20 was originally used, however, version 3.1 should also work. C++17 is required for building the project.
## Testing
Tests' source code is located under src/tests directory. To build tests add `BUILD_TESTS = TRUE` flag when generating build files.
Note that integration_test requires path to project folder as a parameter to access test ROMs. Tests ROMs must be located in `src\tests\integration\roms` directory.  
  
Currently, an error with message `"Reached infinite loop"` is reported when running test ROMs. Such behaviour is expected since test ROMs typically jump to the infinite loop when they are done. This means that this error can occur as a result of normal execution and may signify the end of the test (actually, the infinite loop check is mainly used to terminate emulation when tests are finished). 
## TODO list
