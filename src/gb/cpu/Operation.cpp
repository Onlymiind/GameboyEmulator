#include "Operation.h"

#include <iostream>

namespace gb::decoding {

    std::ostream& operator<<(std::ostream& os, InstructionType type) {
    #define CASE_TYPE(x) case UnprefixedType::##x: os << #x; break
        switch(type) {
            case InstructionType::None: os << "None"; break;
            case InstructionType::NOP: os << "NOP"; break;
            case InstructionType::LD: os << "LD"; break;
            case InstructionType::INC: os << "INC"; break;
            case InstructionType::RLA: os << "RLA"; break;
            case InstructionType::RLCA: os << "RLCA"; break;
            case InstructionType::ADD: os << "ADD"; break;
            case InstructionType::JR: os << "JR"; break;
            case InstructionType::DEC: os << "DEC"; break;
            case InstructionType::RRA: os << "RRA"; break;
            case InstructionType::RRCA: os << "RRCA"; break;
            case InstructionType::SUB: os << "SUB"; break;
            case InstructionType::OR: os << "OR"; break;
            case InstructionType::AND: os << "AND"; break;
            case InstructionType::XOR: os << "XOR"; break;
            case InstructionType::PUSH: os << "PUSH"; break;
            case InstructionType::ADC: os << "ADC"; break;
            case InstructionType::JP: os << "JP"; break;
            case InstructionType::POP: os << "POP"; break;
            case InstructionType::RST: os << "RST"; break;
            case InstructionType::CALL: os << "CALL"; break;
            case InstructionType::SBC: os << "SBC"; break;
            case InstructionType::DI: os << "DI"; break;
            case InstructionType::RET: os << "RET"; break;
            case InstructionType::CPL: os << "CPL"; break;
            case InstructionType::RETI: os << "RETI"; break;
            case InstructionType::CCF: os << "CCF"; break;
            case InstructionType::EI: os << "EI"; break;
            case InstructionType::DAA: os << "DAA"; break;
            case InstructionType::SCF: os << "SCF"; break;
            case InstructionType::HALT: os << "HALT"; break;
            case InstructionType::CP: os << "CP"; break;
            case InstructionType::STOP: os << "STOP"; break;
        }
    return os;
    #undef CASE_TYPE
    }

    std::ostream& operator<<(std::ostream& os, PrefixedType type) {
        switch(type) {
            case PrefixedType::RLC: os << "RLC"; break;
            case PrefixedType::RRC: os << "RRC"; break;
            case PrefixedType::SLA: os << "SLA"; break;
            case PrefixedType::SRL: os << "SRL"; break;
            case PrefixedType::BIT: os << "BIT"; break;
            case PrefixedType::RES: os << "RES"; break;
            case PrefixedType::SET: os << "SET"; break;
            case PrefixedType::RL: os << "RL"; break;
            case PrefixedType::RR: os << "RR"; break;
            case PrefixedType::SWAP: os << "SWAP"; break;
        }
        return os;
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
