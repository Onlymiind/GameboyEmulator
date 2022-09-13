#include "tests/integration/IntegrationTest.h"

#include <filesystem>
#include <string>
#include <iostream>
#include <cassert>


int main(int argc, const char** argv) {
    namespace  fs = std::filesystem;
    fs::path rom_path = fs::current_path() / "src" / "tests" / "integration" / "roms";

    if(!fs::exists(rom_path)) {
        std::cout << "Project path is invalid or directory" << rom_path << " doesn't exist\n" << std::endl;
        return 1;
    }
    fs::current_path(rom_path);

    assert(argc == 2);
    
    return runTestRom(argv[1], std::cout);
}