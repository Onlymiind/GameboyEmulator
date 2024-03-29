#ifndef GB_EMULATOR_SRC_GB_CPU_DECODER_HDR_
#define GB_EMULATOR_SRC_GB_CPU_DECODER_HDR_

#include "gb/cpu/operation.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

/*
Decoding is done according to Game Boy's CPU opcode table and this document:
https://gb-archive.github.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html
*/
namespace gb::cpu {

    DecodedInstruction decodeUnprefixed(Opcode code);
    DecodedInstruction decodePrefixed(Opcode code);

    inline bool isPrefix(Opcode code) { return code.code == 0xCB; }
} // namespace gb::cpu

#endif
