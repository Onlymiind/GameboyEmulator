#include "core/gb/cpu/Decoder.h"
#include "core/gb/cpu/Decoder.cpp"

#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

const std::vector<std::pair<uint8_t, gb::UnprefixedInstruction>> g_UnprefixedSample = 
{

};

void TestPrefix()
{
    gb::Decoder decoder;

    assert(decoder.isPrefix({0xCB}));
}

void TestDecodingPrefixed()
{
    std::cout << "Testing decoding prefixed instructions" << std::endl;
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

    std::cout << "OK" << std::endl;
}

int main()
{
    TestPrefix();
    TestDecodingPrefixed();
    return 0;
}