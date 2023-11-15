#include "gb/emulator.h"
#include "gb/interrupt_register.h"
#include "gb/timer.h"
#include <cstdint>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch2/catch_test_macros.hpp"

#include <iostream>

// TODO: write tests for timer bugs

TEST_CASE("reading and writing") {
    gb::InterruptRegister reg;
    gb::Timer timer(reg);

    REQUIRE(timer.read(uint16_t(gb::IO::DIV)) == 0xAB);
    REQUIRE(timer.read(uint16_t(gb::IO::TIMA)) == 0);
    REQUIRE(timer.read(uint16_t(gb::IO::TMA)) == 0);
    REQUIRE(timer.read(uint16_t(gb::IO::TAC)) == 0xf8);

    timer.write(uint16_t(gb::IO::DIV), 0x10);
    REQUIRE(timer.read(uint16_t(gb::IO::DIV)) == 0);

    timer.write(uint16_t(gb::IO::TIMA), 0x11);
    REQUIRE(timer.read(uint16_t(gb::IO::TIMA)) == 0x11);

    timer.write(uint16_t(gb::IO::TMA), 0x12);
    assert(timer.read(uint16_t(gb::IO::TMA)) == 0x12);

    timer.write(uint16_t(gb::IO::TAC), 0b11100101);
    REQUIRE(timer.read(uint16_t(gb::IO::TAC)) == (0xf8 | 0b00000101));
}

TEST_CASE("interrupt") {
    gb::InterruptRegister reg;
    gb::Timer timer(reg);
    timer.write(uint16_t(gb::IO::DIV), 0);
    timer.write(uint16_t(gb::IO::TAC), 4);

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
        timer.write(uint16_t(gb::IO::DIV), 0);
        timer.write(uint16_t(gb::IO::TAC), 4);

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
        timer.write(uint16_t(gb::IO::DIV), 0);
        timer.write(uint16_t(gb::IO::TAC), 5);

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
        timer.write(uint16_t(gb::IO::DIV), 0);
        timer.write(uint16_t(gb::IO::TAC), 6);

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
        timer.write(uint16_t(gb::IO::DIV), 0);
        timer.write(uint16_t(gb::IO::TAC), 7);

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
    emulator.getTimer().write(uint16_t(gb::IO::DIV), 0);
    REQUIRE(emulator.getTimer().read(uint16_t(gb::IO::DIV)) == 0);
    for (int i = 0; i < 64; ++i) {
        emulator.tick();
    }

    REQUIRE(emulator.getTimer().read(uint16_t(gb::IO::DIV)) == 1);
}
