#include "core/gb/cpu/Decoder.h"

#include <cassert>

void TestPrefix()
{
    gb::Decoder decoder;

    assert(decoder.isPrefix({0xCB}));
}

void TestDecodingPrefixed()
{
    gb::Decoder decoder;

    gb::Instr sample = 
    {
        {},{},{},
        {gb::ArgumentSource::Register, gb::ArgumentType::Unsigned8}
    };
}

int main()
{
    return 0;
}