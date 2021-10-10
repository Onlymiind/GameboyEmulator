#pragma once
#include "gb/cpu/CPUUtils.h"

namespace gb
{
    namespace cpu
    {
        inline void addByte(InstructionContext& context);
        void addSP(InstructionContext& context);
        void addHL(InstructionContext& context);
        void adc(InstructionContext& context);
        void sub(InstructionContext& context);
        void sbc(InstructionContext& context);
        void decByte(InstructionContext& context);
        void decWord(InstructionContext& context);
        void incByte(InstructionContext& context);
        void incWord(InstructionContext& context);
        void and(InstructionContext& context);
        void xor(InstructionContext& context);
        void or(InstructionContext& context);
        void cp(InstructionContext& context);
        void daa(InstructionContext& context);
        void cpl(InstructionContext& context);
        void ccf(InstructionContext& context);
        void scf(InstructionContext& context);
    }
}