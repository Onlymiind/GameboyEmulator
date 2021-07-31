#include "core/gb/cpu/Decoder.h"
#include "core/gb/cpu/Decoder.cpp"

#include "Common.h"

#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
#include <cstdint>
#include <iomanip>

const std::vector<std::pair<uint8_t, gb::UnprefixedInstruction>> g_UnprefixedSample = 
{
    {0x00, {{},{},{},{},{}, gb::InstructionType::NOP}},
    {0x40, {{}, gb::LoadSubtype::Typical, {}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8, gb::Registers::B}, {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8, gb::Registers::B}, gb::InstructionType::LD}},
    {0x82, {}},
    {0xC6, {}},
    {0xE0, {}},
    {0x31, {}},
    {0xC2, {}},
    {0xFF, {}},
    {0x18, {}},
    {0x76, {}}
};

void TestPrefix()
{
    gb::Decoder decoder;

    assert(decoder.isPrefix({0xCB}));
}

void TestDecodingPrefixed()
{
    gb::Decoder decoder;

    gb::PrefixedInstruction RLC_B = { gb::InstructionType::RLC, gb::Registers::B};
    uint8_t RLC_B_code = 0x00;
    gb::PrefixedInstruction SRA_A = { gb::InstructionType::SRA, gb::Registers::A};
    uint8_t SRA_A_code = 0x2F;
    gb::PrefixedInstruction BIT_6_A = {gb::InstructionType::BIT, gb::Registers::A, 6};
    uint8_t BIT_6_A_code = 0x77;
    gb::PrefixedInstruction SET_0_B = {gb::InstructionType::SET, gb::Registers::B, 0};
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
        if(instr.Type != gb::InstructionType::None)
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
    return 0;
}