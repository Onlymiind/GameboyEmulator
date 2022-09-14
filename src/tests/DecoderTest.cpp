#include "gb/cpu/Decoder.h"

#include "Common.h"

#include <cassert>
#include <iostream>
#include <utility>
#include <array>
#include <cstdint>
#include <iomanip>

using namespace gb::decoding;

constexpr std::array<std::pair<uint8_t, UnprefixedInstruction>, 10> unprefixed_sample = 
{{
    {0x00, {{},{},{},{},{}, UnprefixedType::NOP}},
    {0x40, {{}, LoadSubtype::Typical, {}, {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::B}, {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::B}, UnprefixedType::LD}},
    {0x82, {{}, {}, {}, {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::D}, {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A}, UnprefixedType::ADD}},
    {0xC6, {{}, {}, {}, {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None}, {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A}, UnprefixedType::ADD}},
    {0xE0, {{}, LoadSubtype::LD_IO, {}, {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A}, {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None}, UnprefixedType::LD}},
    {0x31, {{}, LoadSubtype::Typical, {}, {ArgumentSource::Immediate, ArgumentType::Unsigned16, Registers::None}, {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP}, UnprefixedType::LD}},
    {0xC2, {{}, {}, Conditions::NotZero, {ArgumentSource::Immediate, ArgumentType::Unsigned16, Registers::None}, {}, UnprefixedType::JP}},
    {0xFF, {0x38, {}, {}, {}, {}, UnprefixedType::RST}},
    {0x18, {{}, {}, {}, {ArgumentSource::Immediate, ArgumentType::Signed8, Registers::None}, {}, UnprefixedType::JR}},
    {0x76, {{}, {}, {}, {}, {}, UnprefixedType::HALT}}
}};

void TestPrefix()
{
    Decoder decoder;

    assert(decoder.isPrefix({0xCB}));
}

void TestDecodingPrefixed()
{
    Decoder decoder;

    PrefixedInstruction RLC_B = { PrefixedType::RLC, Registers::B};
    uint8_t RLC_B_code = 0x00;
    PrefixedInstruction SRA_A = { PrefixedType::SRA, Registers::A};
    uint8_t SRA_A_code = 0x2F;
    PrefixedInstruction BIT_6_A = {PrefixedType::BIT, Registers::A, 6};
    uint8_t BIT_6_A_code = 0x77;
    PrefixedInstruction SET_0_B = {PrefixedType::SET, Registers::B, 0};
    uint8_t SET_0_B_code = 0xC0;

    assert(decoder.decodePrefixed(RLC_B_code) == RLC_B);
    assert(decoder.decodePrefixed(SRA_A_code) == SRA_A);
    assert(decoder.decodePrefixed(BIT_6_A_code) == BIT_6_A);
    assert(decoder.decodePrefixed(SET_0_B_code) == SET_0_B);
}

void TestDecodingUnprefixed()
{
    Decoder decoder;

    for(const auto& [code, instr] : unprefixed_sample)
    {
        if(instr.type != UnprefixedType::None)
        {
            std::cout << "Testing instruction: " << std::hex << "0x" << std::setfill('0') << std::setw(sizeof(uint8_t) * 2) << +code << std::endl;
            assert(decoder.decodeUnprefixed(code) == instr);
        }
    }
}

int main()
{
    RUN_TEST(TestPrefix);
    RUN_TEST(TestDecodingPrefixed);
    RUN_TEST(TestDecodingUnprefixed);
    std::cout << "Done\n";
    return 0;
}