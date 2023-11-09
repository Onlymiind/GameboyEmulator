#include "gb/emulator.h"
#include "util/util.h"

#include "catch2/catch_message.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

std::string run_cmd = "-run ";

class TestOutputReader : public gb::IMemoryObserver {
  public:
    TestOutputReader(std::ostream &out) : out_(out) {}

    void onWrite(uint16_t address, uint8_t data) noexcept override {
        // Used to get output from blargg's test ROMs.
        if (address == 0xFF01) {
            symbol_ = data;
        } else if (address == 0xFF02 && data == 0x81) {
            out_ << symbol_;
        }
    }

    uint16_t minAddress() const noexcept override { return 0xFF01; }
    uint16_t maxAddress() const noexcept override { return 0xFF02; }

  private:
    std::ostream &out_;
    uint8_t symbol_ = 0;
};

const std::string rom_dir = "blargg_test_roms/";

TEST_CASE("run cpu test roms") {
    auto rom_name = GENERATE(as<std::string>{}, "01-special.gb", "02-interrupts.gb", "03-op sp,hl.gb", "04-op r,imm.gb",
                             "05-op rp.gb", "06-ld r,r.gb", "07-jr,jp,call,ret,rst.gb", "08-misc instrs.gb",
                             "09-op r,r.gb", "10-bit ops.gb", "11-op a,(hl).gb", "instr_timing.gb", "01-read_timing.gb",
                             "02-write_timing.gb", "03-modify_timing.gb");

    std::stringstream out;
    TestOutputReader test_out(out);

    gb::Emulator emulator;
    REQUIRE(std::filesystem::exists(rom_dir + rom_name));
    emulator.getCartridge().setROM(readFile(rom_dir + rom_name));

    emulator.getBus().setObserver(test_out);
    emulator.start();
    uint16_t old_pc = emulator.getCPU().getProgramCounter();
    while (!emulator.terminated()) {
        try {
            emulator.tick();
            if (emulator.getCPU().isFinished()) {
                // tests jump to infinite loop after comletion
                if (!emulator.getCPU().isHalted() && emulator.getCPU().getProgramCounter() == old_pc) {
                    INFO("Test rom completion detected at address:");
                    INFO(old_pc);
                    break;
                }
                old_pc = emulator.getCPU().getProgramCounter();
            }

        } catch (const std::exception &e) {
            INFO(e.what());
            break;
        }
    }

    std::string output = out.str();

    INFO(output);
    REQUIRE(output.find("Passed") != std::string::npos);
}
