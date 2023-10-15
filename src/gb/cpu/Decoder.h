#pragma once
#include "gb/cpu/Operation.h"

#include <cstdint>
#include <cstddef>
#include <array>
#include <unordered_map>

/*
Decoding is done according to Game Boy's CPU opcode table and this document:
https://gb-archive.github.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
*/
namespace gb::cpu {

    DecodedInstruction decodeUnprefixed(opcode code);
    DecodedInstruction decodePrefixed(opcode code);

    inline bool isPrefix(opcode code) { return code.code == 0xCB; }
}

