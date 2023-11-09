#include "gb/cpu/decoder.h"
#include "gb/cpu/operation.h"

#include "catch2/catch_test_macros.hpp"

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <utility>

using namespace gb::cpu;

constexpr std::array<std::pair<uint8_t, DecodedInstruction>, 10> unprefixed_sample = {
    {{0x00, {.type = InstructionType::NOP}},
     {0x40,
      {.ld_subtype = LoadSubtype::TYPICAL,
       .src = {ArgumentSource::REGISTER, Registers::B},
       .dst = {ArgumentSource::REGISTER, Registers::B},
       .type = InstructionType::LD}},
     {0x82,
      {.src = {ArgumentSource::REGISTER, Registers::D},
       .dst = {ArgumentSource::REGISTER, Registers::A},
       .type = InstructionType::ADD}},
     {0xC6,
      {.src = {ArgumentSource::IMMEDIATE_U8, Registers::NONE},
       .dst = {ArgumentSource::REGISTER, Registers::A},
       .type = InstructionType::ADD}},
     {0xE0,
      {.ld_subtype = LoadSubtype::LD_IO,
       .src = {ArgumentSource::REGISTER, Registers::A},
       .dst = {ArgumentSource::IMMEDIATE_U8, Registers::NONE},
       .type = InstructionType::LD}},
     {0x31,
      {.ld_subtype = LoadSubtype::TYPICAL,
       .src = {ArgumentSource::IMMEDIATE_U16, Registers::NONE},
       .dst = {ArgumentSource::DOUBLE_REGISTER, Registers::SP},
       .type = InstructionType::LD}},
     {0xC2,
      {.condition = Conditions::NOT_ZERO,
       .src = {ArgumentSource::IMMEDIATE_U16, Registers::NONE},
       .type = InstructionType::JP}},
     {0xFF, {.reset_vector = 0x38, .type = InstructionType::RST}},
     {0x18, {.src = {ArgumentSource::IMMEDIATE_S8, Registers::NONE}, .type = InstructionType::JR}},
     {0x76, {.type = InstructionType::HALT}}}};

TEST_CASE("prefix") { REQUIRE(isPrefix({0xCB})); }

TEST_CASE("decoding prefixed instructions") {
    DecodedInstruction RLC_B = {
        .src = {.src = ArgumentSource::REGISTER, .reg = Registers::B},
        .type = InstructionType::RLC,
    };
    uint8_t RLC_B_code = 0x00;
    DecodedInstruction SRA_A = {
        .src = {.src = ArgumentSource::REGISTER, .reg = Registers::A},
        .type = InstructionType::SRA,
    };
    uint8_t SRA_A_code = 0x2F;
    DecodedInstruction BIT_6_A = {.src = {.src = ArgumentSource::REGISTER, .reg = Registers::A},
                                  .type = InstructionType::BIT,
                                  .bit = 6};
    uint8_t BIT_6_A_code = 0x77;
    DecodedInstruction SET_0_B = {.src = {.src = ArgumentSource::REGISTER, .reg = Registers::B},
                                  .type = InstructionType::SET,
                                  .bit = 0};
    uint8_t SET_0_B_code = 0xC0;

    REQUIRE(decodePrefixed(RLC_B_code) == RLC_B);
    REQUIRE(decodePrefixed(SRA_A_code) == SRA_A);
    REQUIRE(decodePrefixed(BIT_6_A_code) == BIT_6_A);
    REQUIRE(decodePrefixed(SET_0_B_code) == SET_0_B);
}

TEST_CASE("decoding unprefixed instructions") {

    for (const auto &[code, instr] : unprefixed_sample) {
        if (instr.type != InstructionType::NONE) {
            std::cout << "Testing instruction: " << std::hex << "0x" << std::setfill('0')
                      << std::setw(sizeof(uint8_t) * 2) << +code << std::endl;
            auto result = decodeUnprefixed(code);
            REQUIRE(result == instr);
        }
    }
}
