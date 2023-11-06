#include "Operation.h"

#include <iostream>

namespace gb::cpu {
#define CASE(x)                                                                                                        \
    case ENUM_NAME::x: return #x

    std::string_view to_string(InstructionType type) {
#define ENUM_NAME InstructionType
        switch (type) {
            CASE(NONE);
            CASE(NOP);
            CASE(LD);
            CASE(INC);
            CASE(RLA);
            CASE(RLCA);
            CASE(ADD);
            CASE(JR);
            CASE(DEC);
            CASE(RRA);
            CASE(RRCA);
            CASE(SUB);
            CASE(OR);
            CASE(AND);
            CASE(XOR);
            CASE(PUSH);
            CASE(ADC);
            CASE(JP);
            CASE(POP);
            CASE(RST);
            CASE(CALL);
            CASE(SBC);
            CASE(DI);
            CASE(RET);
            CASE(CPL);
            CASE(RETI);
            CASE(CCF);
            CASE(EI);
            CASE(DAA);
            CASE(SCF);
            CASE(HALT);
            CASE(CP);
            CASE(STOP);
            CASE(RLC);
            CASE(RRC);
            CASE(SLA);
            CASE(SRA);
            CASE(SRL);
            CASE(BIT);
            CASE(RES);
            CASE(SET);
            CASE(RL);
            CASE(RR);
            CASE(SWAP);
        }
#undef ENUM_NAME
    }

    std::string_view to_string(Registers reg) {
#define ENUM_NAME Registers
        switch (reg) {
            CASE(A);
            CASE(B);
            CASE(C);
            CASE(D);
            CASE(E);
            CASE(H);
            CASE(L);
            CASE(AF);
            CASE(BC);
            CASE(DE);
            CASE(SP);
            CASE(PC);
            CASE(HL);
            CASE(NONE);
        default: return "";
        }
#undef ENUM_NAME
    }

    std::string_view to_string(Conditions cond) {
        using enum Conditions;
        switch (cond) {
        case NOT_ZERO: return "nz";
        case ZERO: return "z";
        case NOT_CARRY: return "nc";
        case CARRY: return "c";
        default: return "";
        }
    }

    // TODO
    //  std::ostream& operator<<(std::ostream& os, LoadSubtype subtype);
    //  std::ostream& operator<<(std::ostream& os, ArgumentSource source);
    //  std::ostream& operator<<(std::ostream& os, ArgumentType arg_type);
    //  std::ostream& operator<<(std::ostream& os, Conditions condition);
    //  std::ostream& operator<<(std::ostream& os, ArgumentInfo arg_info);
    //  std::ostream& operator<<(std::ostream& os, UnprefixedInstruction instr);
    //  std::ostream& operator<<(std::ostream& os, PrefixedInstruction instr);
} // namespace gb::cpu
