#include "gb/cpu/Decoder.h"
#include "gb/cpu/Operation.h"

#include "catch2/catch_test_macros.hpp"

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <utility>

using namespace gb::cpu;

constexpr std::array<std::pair<uint8_t, DecodedInstruction>, 10> unprefixed_sample = {
    {{0x00, {{}, {}, {}, {}, {}, InstructionType::NOP}},
     {0x40,
      {{},
       LoadSubtype::Typical,
       {},
       {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::B},
       {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::B},
       InstructionType::LD}},
     {0x82,
      {{},
       {},
       {},
       {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::D},
       {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
       InstructionType::ADD}},
     {0xC6,
      {{},
       {},
       {},
       {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None},
       {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
       InstructionType::ADD}},
     {0xE0,
      {{},
       LoadSubtype::LD_IO,
       {},
       {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
       {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None},
       InstructionType::LD}},
     {0x31,
      {{},
       LoadSubtype::Typical,
       {},
       {ArgumentSource::Immediate, ArgumentType::Unsigned16, Registers::None},
       {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP},
       InstructionType::LD}},
     {0xC2,
      {{},
       {},
       Conditions::NotZero,
       {ArgumentSource::Immediate, ArgumentType::Unsigned16, Registers::None},
       {},
       InstructionType::JP}},
     {0xFF, {0x38, {}, {}, {}, {}, InstructionType::RST}},
     {0x18,
      {{},
       {},
       {},
       {ArgumentSource::Immediate, ArgumentType::Signed8, Registers::None},
       {},
       InstructionType::JR}},
     {0x76, {{}, {}, {}, {}, {}, InstructionType::HALT}}}};

TEST_CASE("prefix") { REQUIRE(isPrefix({0xCB})); }

TEST_CASE("decoding prefixed instructions") {
    DecodedInstruction RLC_B = {
        .src = {.src = ArgumentSource::Register,
                .type = ArgumentType::Unsigned8,
                .reg = Registers::B},
        .type = InstructionType::RLC,
    };
    uint8_t RLC_B_code = 0x00;
    DecodedInstruction SRA_A = {
        .src = {.src = ArgumentSource::Register,
                .type = ArgumentType::Unsigned8,
                .reg = Registers::A},
        .type = InstructionType::SRA,
    };
    uint8_t SRA_A_code = 0x2F;
    DecodedInstruction BIT_6_A = {.src = {.src = ArgumentSource::Register,
                                          .type = ArgumentType::Unsigned8,
                                          .reg = Registers::A},
                                  .type = InstructionType::BIT,
                                  .bit = 6};
    uint8_t BIT_6_A_code = 0x77;
    DecodedInstruction SET_0_B = {.src = {.src = ArgumentSource::Register,
                                          .type = ArgumentType::Unsigned8,
                                          .reg = Registers::B},
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
        if (instr.type != InstructionType::None) {
            std::cout << "Testing instruction: " << std::hex << "0x" << std::setfill('0')
                      << std::setw(sizeof(uint8_t) * 2) << +code << std::endl;
            REQUIRE(decodeUnprefixed(code) == instr);
        }
    }
}
