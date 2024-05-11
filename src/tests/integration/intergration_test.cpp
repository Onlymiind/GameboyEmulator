#include "gb/emulator.h"
#include "gb/memory/breakpoint.h"
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

const std::string rom_dir = "blargg_test_roms/";

TEST_CASE("run cpu test roms") {
    auto rom_name = GENERATE(as<std::string>{}, "01-special.gb",
                             "02-interrupts.gb", "03-op sp,hl.gb",
                             "04-op r,imm.gb", "05-op rp.gb", "06-ld r,r.gb",
                             "07-jr,jp,call,ret,rst.gb", "08-misc instrs.gb",
                             "09-op r,r.gb", "10-bit ops.gb", "11-op a,(hl).gb",
                             "instr_timing.gb", "01-read_timing.gb",
                             "02-write_timing.gb", "03-modify_timing.gb");

    std::stringstream out;
    uint8_t chr{};

    gb::Emulator emulator;
    REQUIRE(std::filesystem::exists(rom_dir + rom_name));
    emulator.getCartridge().setROM(readFile(rom_dir + rom_name));
    emulator.getBus().setBreakpointCallback(
        [&out, &chr](uint16_t address, gb::StopReason, uint8_t data) {
            if (address == 0xff01) {
                chr = data;
            } else if (address == 0xff02) {
                out << chr;
            }
        });
    emulator.getBus().setBreakpoint(0xff01,
                                    gb::Breakpoint{
                                        .info = gb::BreakpointInfo::WRITE});
    emulator.getBus()
        .setBreakpoint(0xff02,
                       gb::Breakpoint{.info = gb::BreakpointInfo::WRITE |
                                              gb::BreakpointInfo::HAS_VALUE,
                                      .data = 0x81});
    emulator.reset();
    emulator.start();
    uint16_t old_pc = 0xffff;
    while (!emulator.terminated()) {
        try {
            emulator.tick();
            if (emulator.getCPU().isFinished()) {
                // tests jump to infinite loop after comletion
                if (!emulator.getCPU().isHalted() &&
                    emulator.getCPU().getLastInstruction().registers.pc() ==
                        old_pc) {
                    INFO("Test rom completion detected at address:");
                    INFO(old_pc);
                    break;
                }
                old_pc = emulator.getCPU().getLastInstruction().registers.pc();
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
