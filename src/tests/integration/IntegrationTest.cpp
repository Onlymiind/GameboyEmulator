#include "gb/Emulator.h"
#include "gb/memory/Memory.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "utils/Utils.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <sstream>
#include <memory>

std::string run_cmd = "-run ";

class TestOutputReader : public gb::MemoryObject
{
public:
    TestOutputReader(std::ostream& out)
        : out_(out)
    {}

    uint8_t read(uint16_t address) const override {
        return 0;
    }

    void write(uint16_t address, uint8_t data) override {
        //Used to get output from blargg's test ROMs.
        if(address == 0) {
            symbol = data;
        } else if (address == 0x01 && data == 0x81) {
            out_ << symbol;
        }
    }
private:
    std::ostream& out_;
    uint8_t symbol = 0;
};

TEST_CASE("run cpu test roms") {
    auto rom_name  = GENERATE(as<std::string>{}, 
        "01-special.gb",
        "02-interrupts.gb",
        "03-op sp,hl.gb",
        "04-op r,imm.gb",
        "05-op rp.gb",
        "06-ld r,r.gb",
        "07-jr,jp,call,ret,rst.gb",
        "08-misc instrs.gb",
        "09-op r,r.gb",
        "10-bit ops.gb",
        "11-op a,(hl).gb"
    );

    std::stringstream out;
    TestOutputReader test_out(out);

    gb::Emulator emulator;
    REQUIRE(std::filesystem::exists(rom_name));
    emulator.setROM(readFile(rom_name));

    emulator.addMemoryObserver({0xFF01, 0xFF02, test_out});
    emulator.start();
    while(!emulator.terminated()) {
        emulator.tick();
    }

    std::string output = out.str();

    INFO(output);
    REQUIRE(output.find("Passed") != std::string::npos);
}
