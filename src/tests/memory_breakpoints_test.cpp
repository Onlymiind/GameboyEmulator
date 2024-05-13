#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "catch2/generators/catch_generators_range.hpp"

#include "gb/emulator.h"
#include "gb/memory/breakpoint.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <set>

static std::vector<std::pair<uint16_t, gb::Breakpoint>> cases{
    {0, gb::Breakpoint{.info = gb::BreakpointInfo::FETCH}},
    {1, gb::Breakpoint{.info = gb::BreakpointInfo::READ}},
    {2, gb::Breakpoint{.info = gb::BreakpointInfo::WRITE}},
    {3, gb::Breakpoint{.info = gb::BreakpointInfo::FETCH |
                               gb::BreakpointInfo::HAS_VALUE,
                       .data = 1}},
    {4, gb::Breakpoint{.info = gb::BreakpointInfo::READ |
                               gb::BreakpointInfo::HAS_VALUE,
                       .data = 1}},
    {5, gb::Breakpoint{.info = gb::BreakpointInfo::WRITE |
                               gb::BreakpointInfo::HAS_VALUE,
                       .data = 1}},
    {5, gb::Breakpoint{.info = gb::BreakpointInfo::WRITE |
                               gb::BreakpointInfo::READ |
                               gb::BreakpointInfo::FETCH |

                               gb::BreakpointInfo::HAS_VALUE,
                       .data = 1}},
};

TEST_CASE("breakpoints: hitting") {
    size_t idx = GENERATE(Catch::Generators::range(size_t(0), cases.size()));

    constexpr uint8_t value = 1;
    auto [address, info] = cases[idx];

    gb::Emulator emulator;
    bool hit = false;
    auto callback = [&hit](auto, auto, auto) { hit = true; };
    emulator.getBus().setBreakpointCallback(callback);

    // minimum ROM size is 32 kilobytes
    // This is not needed by the tests,
    // but it is better to add a full ROM just in case
    std::vector<uint8_t> dummy_rom;
    dummy_rom.resize(32 * 1024);
    dummy_rom[address] = value;
    emulator.getCartridge().setROM(std::move(dummy_rom));

    emulator.getBus().setBreakpoint(address, info);

    using enum gb::BreakpointInfo;
    if (!!(info.info & READ)) {
        emulator.getBus().read(address);
        REQUIRE(hit);
        hit = false;
    }
    if (!!(info.info & FETCH)) {
        emulator.getBus().fetch(address);
        REQUIRE(hit);
        hit = false;
    }
    if (!!(info.info & WRITE)) {
        emulator.getBus().write(address, value);
        REQUIRE(hit);
        hit = false;
    }
}

TEST_CASE("breakpoints: not hitting") {
    size_t idx = GENERATE(Catch::Generators::range(size_t(0), cases.size()));

    constexpr uint8_t wrong_value = 2;
    constexpr uint8_t value = 1;
    auto [address, info] = cases[idx];

    gb::Emulator emulator;
    bool hit = false;
    auto callback = [&hit](auto, auto, auto) { hit = true; };
    emulator.getBus().setBreakpointCallback(callback);

    // minimum ROM size is 32 kilobytes
    // This is not needed by the tests,
    // but it is better to add a full ROM just in case
    std::vector<uint8_t> dummy_rom;
    dummy_rom.resize(32 * 1024);
    dummy_rom[address] = wrong_value;
    emulator.getCartridge().setROM(std::move(dummy_rom));

    emulator.getBus().setBreakpoint(address, info);

    uint16_t other_address = address + 1;

    using enum gb::BreakpointInfo;
    if (!!(info.info & READ)) {
        emulator.getBus().read(other_address);
        REQUIRE(!hit);
        hit = false;
        if (!!(info.info & HAS_VALUE)) {
            emulator.getBus().read(address);
            REQUIRE(!hit);
            hit = false;
        }
    }
    if (!!(info.info & FETCH)) {
        emulator.getBus().fetch(other_address);
        REQUIRE(!hit);
        hit = false;
        if (!!(info.info & HAS_VALUE)) {
            emulator.getBus().fetch(address);
            REQUIRE(!hit);
            hit = false;
        }
    }
    if (!!(info.info & WRITE)) {
        emulator.getBus().write(other_address, value);
        REQUIRE(!hit);
        hit = false;
        if (!!(info.info & HAS_VALUE)) {
            emulator.getBus().write(address, wrong_value);
            REQUIRE(!hit);
            hit = false;
        }
    }
}
