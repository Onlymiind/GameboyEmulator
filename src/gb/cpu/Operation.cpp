#include "Operation.h"

#include <iostream>

namespace gb::decoding {

    std::ostream& operator<<(std::ostream& os, UnprefixedType type) {
    #define CASE_TYPE(x) case UnprefixedType::##x: os << #x; break
        switch(type) {
            case UnprefixedType::None: os << "None"; break;
            case UnprefixedType::NOP: os << "NOP"; break;
            case UnprefixedType::LD: os << "LD"; break;
            case UnprefixedType::INC: os << "INC"; break;
            case UnprefixedType::RLA: os << "RLA"; break;
            case UnprefixedType::RLCA: os << "RLCA"; break;
            case UnprefixedType::ADD: os << "ADD"; break;
            case UnprefixedType::JR: os << "JR"; break;
            case UnprefixedType::DEC: os << "DEC"; break;
            case UnprefixedType::RRA: os << "RRA"; break;
            case UnprefixedType::RRCA: os << "RRCA"; break;
            case UnprefixedType::SUB: os << "SUB"; break;
            case UnprefixedType::OR: os << "OR"; break;
            case UnprefixedType::AND: os << "AND"; break;
            case UnprefixedType::XOR: os << "XOR"; break;
            case UnprefixedType::PUSH: os << "PUSH"; break;
            case UnprefixedType::ADC: os << "ADC"; break;
            case UnprefixedType::JP: os << "JP"; break;
            case UnprefixedType::POP: os << "POP"; break;
            case UnprefixedType::RST: os << "RST"; break;
            case UnprefixedType::CALL: os << "CALL"; break;
            case UnprefixedType::SBC: os << "SBC"; break;
            case UnprefixedType::DI: os << "DI"; break;
            case UnprefixedType::RET: os << "RET"; break;
            case UnprefixedType::CPL: os << "CPL"; break;
            case UnprefixedType::RETI: os << "RETI"; break;
            case UnprefixedType::CCF: os << "CCF"; break;
            case UnprefixedType::EI: os << "EI"; break;
            case UnprefixedType::DAA: os << "DAA"; break;
            case UnprefixedType::SCF: os << "SCF"; break;
            case UnprefixedType::HALT: os << "HALT"; break;
            case UnprefixedType::CP: os << "CP"; break;
            case UnprefixedType::STOP: os << "STOP"; break;
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
