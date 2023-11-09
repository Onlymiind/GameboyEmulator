#include "gb/Timer.h"
#include "gb/InterruptRegister.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

#include <iostream>

// TODO: write tests for timer bugs

TEST_CASE("reading and writing") {
    gb::InterruptRegister reg;
    gb::Timer timer(reg);

    REQUIRE(timer.read(gb::g_timer_div_address) == 0xAB);
    REQUIRE(timer.read(gb::g_timer_tima_address) == 0);
    REQUIRE(timer.read(gb::g_timer_tma_address) == 0);
    REQUIRE(timer.read(gb::g_timer_tac_address) == 0xf8);

    timer.write(gb::g_timer_div_address, 0x10);
    REQUIRE(timer.read(gb::g_timer_div_address) == 0);

    timer.write(gb::g_timer_tima_address, 0x11);
    REQUIRE(timer.read(gb::g_timer_tima_address) == 0x11);

    timer.write(gb::g_timer_tma_address, 0x12);
    assert(timer.read(gb::g_timer_tma_address) == 0x12);

    timer.write(gb::g_timer_tac_address, 0b11100101);
    REQUIRE(timer.read(gb::g_timer_tac_address) == (0xf8 | 0b00000101));
}

TEST_CASE("interrupt") {
    gb::InterruptRegister reg;
    gb::Timer timer(reg);
    timer.write(gb::g_timer_div_address, 0);
    timer.write(gb::g_timer_tac_address, 4);

    // This should cause timer with default register values to set interrupt bit
    for (int j = 256; j > 0; --j) {
        for (int i = 1024; i > 0; --i) {
            timer.update();
        }
    }

    REQUIRE((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
}

TEST_CASE("frequencies") {
    {
        gb::InterruptRegister reg;
        gb::Timer timer(reg);

        // Enable timer, set frequency to the first one
        timer.write(gb::g_timer_div_address, 0);
        timer.write(gb::g_timer_tac_address, 4);

        for (int i = 1024 * 256 - 1; i > 0; --i) {
            timer.update();
            REQUIRE(!(reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
        }

        timer.update();
        REQUIRE((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
    }
    {
        gb::InterruptRegister reg;
        gb::Timer timer(reg);

        // Enable timer, set second frequency
        timer.write(gb::g_timer_div_address, 0);
        timer.write(gb::g_timer_tac_address, 5);

        for (int i = 16 * 256 - 1; i > 0; --i) {
            timer.update();
            REQUIRE(!(reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
        }

        timer.update();
        REQUIRE((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
    }
    {
        gb::InterruptRegister reg;
        gb::Timer timer(reg);

        // Enable timer, set third frequency
        timer.write(gb::g_timer_div_address, 0);
        timer.write(gb::g_timer_tac_address, 6);

        for (int i = 64 * 256 - 1; i > 0; --i) {
            timer.update();
            REQUIRE(!(reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
        }

        timer.update();
        REQUIRE((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
    }
    {
        gb::InterruptRegister reg;
        gb::Timer timer(reg);

        // Enable timer, set last frequency
        timer.write(gb::g_timer_div_address, 0);
        timer.write(gb::g_timer_tac_address, 7);

        for (int i = 256 * 256 - 1; i > 0; --i) {
            timer.update();
            REQUIRE(!(reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
        }

        timer.update();
        REQUIRE((reg.getFlags() & static_cast<uint8_t>(gb::InterruptFlags::TIMER)));
    }
}

TEST_CASE("incrementing in emulator") {
    gb::Emulator emulator;
    emulator.getCartridge().setROM(std::vector<uint8_t>(32 * 1024));
    emulator.reset();
    emulator.start();
    emulator.getTimer().write(gb::g_timer_div_address, 0);
    REQUIRE(emulator.getTimer().read(gb::g_timer_div_address) == 0);
    for (int i = 0; i < 64; ++i) {
        emulator.tick();
    }

    REQUIRE(emulator.getTimer().read(gb::g_timer_div_address) == 1);
}
