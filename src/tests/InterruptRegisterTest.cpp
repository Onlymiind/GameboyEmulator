#include "gb/InterruptRegister.h"

#include "catch2/catch_test_macros.hpp"

#include <cstdint>
#include <iostream>

TEST_CASE("interrupt register") {
    gb::InterruptRegister reg;

    REQUIRE(reg.getFlags() == gb::g_unused_interrupt_bits);

    reg.setFlag(gb::InterruptFlags::LCD_STAT);
    REQUIRE(reg.read(0x0000) == reg.getFlags());
    REQUIRE(reg.getFlags() ==
            (gb::g_unused_interrupt_bits | static_cast<uint8_t>(gb::InterruptFlags::LCD_STAT)));

    reg.clearFlag(gb::InterruptFlags::LCD_STAT);
    REQUIRE(reg.getFlags() == gb::g_unused_interrupt_bits);

    reg.clearFlag(gb::InterruptFlags::Joypad);
    REQUIRE(reg.getFlags() == gb::g_unused_interrupt_bits);

    reg.write(0x0000,
              gb::g_unused_interrupt_bits | static_cast<uint8_t>(gb::InterruptFlags::VBlank));
    REQUIRE(reg.read(0x0000) == reg.getFlags());
    REQUIRE(reg.getFlags() ==
            (gb::g_unused_interrupt_bits | static_cast<uint8_t>(gb::InterruptFlags::VBlank)));

    reg.write(0x0000, 0x00);
    REQUIRE(reg.getFlags() == gb::g_unused_interrupt_bits);
}
