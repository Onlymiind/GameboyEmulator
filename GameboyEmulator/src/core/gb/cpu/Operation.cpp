#include "Operation.h"

#include <iostream>

namespace gb 
{
    std::ostream& operator<<(std::ostream& os, UnprefixedType type)
    {
    #define CASE_TYPE(x) case UnprefixedType::##x: os << #x; break
        switch(type)
        {
            CASE_TYPE(None);
            CASE_TYPE(NOP);
            CASE_TYPE(LD);
            CASE_TYPE(INC);
            CASE_TYPE(RLA);
            CASE_TYPE(RLCA);
            CASE_TYPE(ADD);
            CASE_TYPE(JR);
            CASE_TYPE(DEC);
            CASE_TYPE(RRA);
            CASE_TYPE(RRCA);
            CASE_TYPE(SUB);
            CASE_TYPE(OR);
            CASE_TYPE(AND);
            CASE_TYPE(XOR);
            CASE_TYPE(PUSH);
            CASE_TYPE(ADC);
            CASE_TYPE(JP);
            CASE_TYPE(POP);
            CASE_TYPE(RST);
            CASE_TYPE(CALL);
            CASE_TYPE(SBC);
            CASE_TYPE(DI);
            CASE_TYPE(RET);
            CASE_TYPE(CPL);
            CASE_TYPE(RETI);
            CASE_TYPE(CCF);
            CASE_TYPE(EI);
            CASE_TYPE(DAA);
            CASE_TYPE(SCF);
            CASE_TYPE(HALT);
            CASE_TYPE(CP);
            CASE_TYPE(STOP);
        }
    return os;
    #undef CASE_TYPE
    }

    std::ostream& operator<<(std::ostream& os, PrefixedType type)
    {
    #define CASE_TYPE(x) case PrefixedType::##x: os << #x; break
        switch(type)
        {
            CASE_TYPE(RLC);
            CASE_TYPE(RRC);
            CASE_TYPE(SLA);
            CASE_TYPE(SRL);
            CASE_TYPE(BIT);
            CASE_TYPE(RES);
            CASE_TYPE(SET);
            CASE_TYPE(RL);
            CASE_TYPE(RR);
            CASE_TYPE(SWAP);
        }
        return os;
    #undef CASE_TYPE
    }

    //TODO
    // std::ostream& operator<<(std::ostream& os, LoadSubtype subtype);
    // std::ostream& operator<<(std::ostream& os, ArgumentSource source);
    // std::ostream& operator<<(std::ostream& os, ArgumentType arg_type);
    // std::ostream& operator<<(std::ostream& os, Conditions condition);
    // std::ostream& operator<<(std::ostream& os, ArgumentInfo arg_info);
    // std::ostream& operator<<(std::ostream& os, UnprefixedInstruction instr);
    // std::ostream& operator<<(std::ostream& os, PrefixedInstruction instr);
}