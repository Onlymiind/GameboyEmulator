#include "core/gb/cpu/Decoder.h"
#include "core/gb/cpu/Decoder.cpp"

#include "Common.h"

#include <cassert>
#include <iostream>
#include <utility>
#include <array>
#include <cstdint>
#include <iomanip>

constexpr std::array<std::pair<uint8_t, gb::UnprefixedInstruction>, 10> g_UnprefixedSample = 
{{
    {0x00, {{},{},{},{},{}, gb::UnprefixedType::NOP}},
    {0x40, {{}, gb::LoadSubtype::Typical, {}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8, gb::Registers::B}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8, gb::Registers::B}, gb::UnprefixedType::LD}},
    {0x82, {{}, {}, {}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8, gb::Registers::D}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8, gb::Registers::A}, gb::UnprefixedType::ADD}},
    {0xC6, {{}, {}, {}, {gb::ArgumentSource::Immediate, gb::ArgumentType::Unsigned8, gb::Registers::None}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8, gb::Registers::A}, gb::UnprefixedType::ADD}},
    {0xE0, {{}, gb::LoadSubtype::LD_IO, {}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8, gb::Registers::A}, {gb::ArgumentSource::IndirectImmediate, gb::ArgumentType::Unsigned8, gb::Registers::None}, gb::UnprefixedType::LD}},
    {0x31, {{}, gb::LoadSubtype::Typical, {}, {gb::ArgumentSource::Immediate, gb::ArgumentType::Unsigned16, gb::Registers::None}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned16, gb::Registers::SP}, gb::UnprefixedType::LD}},
    {0xC2, {{}, {}, gb::Conditions::NotZero, {gb::ArgumentSource::Immediate, gb::ArgumentType::Unsigned16, gb::Registers::None}, {}, gb::UnprefixedType::JP}},
    {0xFF, {0x38, {}, {}, {}, {}, gb::UnprefixedType::RST}},
    {0x18, {{}, {}, {}, {gb::ArgumentSource::Immediate, gb::ArgumentType::Signed8, gb::Registers::None}, {}, gb::UnprefixedType::JR}},
    {0x76, {{}, {}, {}, {}, {}, gb::UnprefixedType::HALT}}
}};

void TestPrefix()
{
    gb::Decoder decoder;

    assert(decoder.isPrefix({0xCB}));
}

void TestDecodingPrefixed()
{
    gb::Decoder decoder;

    gb::PrefixedInstruction RLC_B = { gb::PrefixedType::RLC, gb::Registers::B};
    uint8_t RLC_B_code = 0x00;
    gb::PrefixedInstruction SRA_A = { gb::PrefixedType::SRA, gb::Registers::A};
    uint8_t SRA_A_code = 0x2F;
    gb::PrefixedInstruction BIT_6_A = {gb::PrefixedType::BIT, gb::Registers::A, 6};
    uint8_t BIT_6_A_code = 0x77;
    gb::PrefixedInstruction SET_0_B = {gb::PrefixedType::SET, gb::Registers::B, 0};
    uint8_t SET_0_B_code = 0xC0;

    assert(decoder.decodePrefixed(RLC_B_code) == RLC_B);
    assert(decoder.decodePrefixed(SRA_A_code) == SRA_A);
    assert(decoder.decodePrefixed(BIT_6_A_code) == BIT_6_A);
    assert(decoder.decodePrefixed(SET_0_B_code) == SET_0_B);
}

void TestDecodingUnprefixed()
{
    gb::Decoder decoder;

    for(const auto& [code, instr] : g_UnprefixedSample)
    {
        if(instr.Type != gb::UnprefixedType::None)
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