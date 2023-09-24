#include "gb/cpu/Decoder.h"

#include <utility>

namespace gb::decoding {

    using type = UnprefixedType;
    using pref_type = PrefixedType;
    using arg_src = ArgumentSource;
    using arg_t = ArgumentType;
    using reg = Registers;

    struct column_hash {
        inline size_t operator()(uint8_t value) const {
            return value & 0b1100'1111;
        }
    };
    struct collideable_equal {
        inline bool operator()(const uint8_t& lhs, const uint8_t& rhs) const {
            return column_hash{}(lhs) == column_hash{}(rhs);
        }
    };

    constexpr std::array<Registers, 8> g_byte_registers_ = {
        Registers::B,  Registers::C,
        Registers::D,  Registers::E,
        Registers::H,  Registers::L,
        Registers::HL, Registers::A 
    };

    //Register pair lookup with SP
    constexpr std::array<Registers, 4> g_word_registers_SP_ = { Registers::BC, Registers::DE, Registers::HL, Registers::SP };

    //Register pair lookup with AF
    constexpr std::array<Registers, 4> g_word_registers_AF_ = { Registers::BC, Registers::DE, Registers::HL, Registers::AF };

    //Conditions lookup
    constexpr std::array<Conditions, 4> g_conditions_ = { 
        Conditions::NotZero,
        Conditions::Zero,
        Conditions::NotCarry,
        Conditions::Carry
    };

    constexpr std::array<UnprefixedType, 8> g_ALU_ = {
        UnprefixedType::ADD, UnprefixedType::ADC, UnprefixedType::SUB, UnprefixedType::SBC,
        UnprefixedType::AND, UnprefixedType::XOR, UnprefixedType::OR,  UnprefixedType::CP
    };

    constexpr std::array<PrefixedType, 8> g_bit_operations_ = {
        PrefixedType::RLC,  PrefixedType::RRC,
        PrefixedType::RL,   PrefixedType::RR,
        PrefixedType::SLA,  PrefixedType::SRA,
        PrefixedType::SWAP, PrefixedType::SRL
    };

    static const std::unordered_map<uint8_t, UnprefixedType, column_hash, collideable_equal> g_columns_ = {
        { 0x01, UnprefixedType::LD }, { 0x02, UnprefixedType::LD }, { 0x03, UnprefixedType::INC },
        { 0x04, UnprefixedType::INC }, { 0x05, UnprefixedType::DEC }, { 0x06, UnprefixedType::LD },
        { 0x09, UnprefixedType::ADD }, { 0x0A, UnprefixedType::LD }, { 0x0B, UnprefixedType::DEC },
        { 0x0C, UnprefixedType::INC }, { 0x0D, UnprefixedType::DEC }, { 0x0E, UnprefixedType::LD },
        { 0xC1, UnprefixedType::POP }, { 0xC5, UnprefixedType::PUSH }, { 0xC7, UnprefixedType::RST },
        { 0xCF, UnprefixedType::RST }
    };

    static const std::unordered_map<uint8_t, UnprefixedType> g_random_instructions_ = {
        { 0x00, UnprefixedType::NOP }, { 0x07, UnprefixedType::RLCA }, { 0x08, UnprefixedType::LD }, { 0x0F, UnprefixedType::RRCA },
        
        { 0x10, UnprefixedType::STOP }, { 0x17, UnprefixedType::RLA }, { 0x18, UnprefixedType::JR }, { 0x1F, UnprefixedType::RRA },
        
        { 0x20, UnprefixedType::JR }, { 0x27, UnprefixedType::DAA }, { 0x28, UnprefixedType::JR }, { 0x2F, UnprefixedType::CPL },
        
        { 0x30, UnprefixedType::JR }, { 0x37, UnprefixedType::SCF }, { 0x38, UnprefixedType::JR }, { 0x3F, UnprefixedType::CCF },
        { 0xC0, UnprefixedType::RET }, { 0xC2, UnprefixedType::JP }, { 0xC3, UnprefixedType::JP }, { 0xC4, UnprefixedType::CALL },
        { 0xC8, UnprefixedType::RET }, { 0xC9, UnprefixedType::RET }, { 0xCA, UnprefixedType::JP }, { 0xCC, UnprefixedType::CALL },
        { 0xCD, UnprefixedType::CALL }, 
        
        { 0xD0, UnprefixedType::RET }, { 0xD2, UnprefixedType::JP }, { 0xD4, UnprefixedType::CALL }, { 0xD8, UnprefixedType::RET },
        { 0xD9, UnprefixedType::RETI }, {0xDA, UnprefixedType::JP }, { 0xDC, UnprefixedType::CALL }, 
        
        { 0xE0, UnprefixedType::LD }, { 0xE2, UnprefixedType::LD }, { 0xE8, UnprefixedType::ADD }, { 0xE9, UnprefixedType::JP }, 
        { 0xEA, UnprefixedType::LD },
        
        { 0xF0, UnprefixedType::LD }, { 0xF2, UnprefixedType::LD }, { 0xF3, UnprefixedType::DI }, { 0xF8, UnprefixedType::LD }, 
        { 0xF9, UnprefixedType::LD }, { 0xFA, UnprefixedType::LD }, { 0xFB, UnprefixedType::EI },
    };

    //Some LD instructions are a pain to decode, so it is done with this lookup table
    static const std::unordered_map<uint8_t, UnprefixedInstruction> g_random_LD_ = {
        {0x08, {
                {}, //Reset vector
                LoadSubtype::LD_SP, //LD Subtype
                {}, //Condition
                {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP}, //Source
                {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None}, //Destination
                UnprefixedType::LD //Instruction type
            }},
        {0xE0, {
                {},
                LoadSubtype::LD_IO,
                {},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None},
                UnprefixedType::LD
            }}, 
        {0xF0, {
                {},
                LoadSubtype::LD_IO,
                {},
                {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                UnprefixedType::LD
            }},
        {0xE2, {
                {},
                LoadSubtype::LD_IO,
                {},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                {ArgumentSource::Indirect, ArgumentType::Unsigned8, Registers::C},
                UnprefixedType::LD
            }},
        {0xF2, {
                {},
                LoadSubtype::LD_IO,
                {},
                {ArgumentSource::Indirect, ArgumentType::Unsigned8, Registers::C},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                UnprefixedType::LD
            }},
        {0xF8, {
                {},
                LoadSubtype::LD_Offset_SP,
                {},
                {ArgumentSource::Immediate, ArgumentType::Signed8, Registers::None},
                {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::HL},
                UnprefixedType::LD
            }},
        {0xF9, {
                {},
                LoadSubtype::Typical,
                {},
                {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::HL},
                {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP},
                UnprefixedType::LD
            }},
        {0xEA, {
                {},
                LoadSubtype::Typical,
                {},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None},
                UnprefixedType::LD
            }},
        {0xFA, {
                {},
                LoadSubtype::Typical,
                {},
                {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                UnprefixedType::LD
            }}
    };

    void setRegisterInfo(uint8_t registerIndex, ArgumentInfo& registerInfo);
    void setALUInfo(opcode code, UnprefixedInstruction& instruction, bool hasImmediate);
    void decodeRandomInstructions(opcode code, UnprefixedInstruction& instruction);
    void decodeADD(opcode code, UnprefixedInstruction& instruction);
    void decodeLD(opcode code, UnprefixedInstruction& instruction);
    void decodeJR(opcode code, UnprefixedInstruction& instruction);
    void decodeJP(opcode code, UnprefixedInstruction& instruction);
    void decodeINC_DEC(opcode code, UnprefixedInstruction& instruction);

    UnprefixedInstruction decodeUnprefixed(opcode code) {
        UnprefixedInstruction result;

        switch (code.getX()) {
            case 1:
                if (code.getZ() == 6 && code.getY() == 6) {
                    result.type = type::HALT;
                } else {
                    result.type = type::LD;
                    result.LD_subtype = LoadSubtype::Typical;
                    setRegisterInfo(code.getY(), result.destination);
                    setRegisterInfo(code.getZ(), result.source);
                }
                break;
            case 2:
                setALUInfo(code, result, false);
                break;
            case 3:
                if (code.getZ() == 6) {
                    setALUInfo(code, result, true);
                }
                break;
        }

        if(result.type == type::None) {
            bool isRandomLD = false;
            if(g_columns_.count(code.code)) {
                result.type = g_columns_.at(code.code);
            } else {
                result.type = g_random_instructions_.at(code.code);
                if(result.type == type::LD) {
                    isRandomLD = true;
                }
            }
            if(!isRandomLD) {
                decodeRandomInstructions(code, result);
            } else {
                result = g_random_LD_.at(code.code);
            }
        }

        return result;
    }

    void decodeRandomInstructions(opcode code, UnprefixedInstruction& instruction) {
        switch(instruction.type) {
            case type::NOP:
            case type::STOP:
            case type::SCF:
            case type::CCF:
            case type::CPL:
            case type::DAA:
            case type::DI:
            case type::EI:
            case type::RETI:
            case type::RLCA:
            case type::RRCA:
            case type::RLA:
            case type::RRA:
                return;
            case type::RST:
                instruction.reset_vector = code.getY() * 8;
                return;
            case type::POP:
                instruction.destination.reg = g_word_registers_AF_[code.getP()];
                return;
            case type::PUSH:
                instruction.source.reg = g_word_registers_AF_[code.getP()];
                return;
            case type::RET:
                if(code.getZ() == 0) {
                    instruction.condition = g_conditions_[code.getY()];
                }
                return;
            case type::ADD:
                decodeADD(code, instruction);
                return;
            case type::JR:
                decodeJR(code, instruction);
                return;
            case type::JP:
                decodeJP(code, instruction);
                return;
            case type::CALL:
                if(code.getZ() == 4) {
                    instruction.condition = g_conditions_[code.getY()];
                }
                return;
            case type::INC:
            case type::DEC:
                decodeINC_DEC(code, instruction);
                return;
            case type::LD:
                decodeLD(code, instruction);
                return;
            default:
                return;
        }
    }

    void decodeADD(opcode code, UnprefixedInstruction& instruction) {
        switch(code.getZ()) {
            case 0:
                instruction.destination.source = arg_src::Register;
                instruction.destination.type = arg_t::Unsigned16;
                instruction.destination.reg = reg::SP;
                instruction.source.source = arg_src::Immediate;
                instruction.source.type = arg_t::Signed8;
                break;
            case 1:
                instruction.destination.source = arg_src::Register;
                instruction.destination.type = arg_t::Unsigned16;
                instruction.destination.reg = reg::HL;
                instruction.source.source = arg_src::Register;
                instruction.source.type = arg_t::Unsigned16;
                instruction.source.reg = g_word_registers_SP_[code.getP()];
                break;
        }
    }

    void decodeLD(opcode code, UnprefixedInstruction& instruction) {
        switch(code.getZ()) {
            case 1:
                instruction.destination.reg = g_word_registers_SP_[code.getP()];
                instruction.destination.source = arg_src::Register;
                instruction.destination.type = arg_t::Unsigned16;
                instruction.source.source = arg_src::Immediate;
                instruction.source.type = arg_t::Unsigned16;
                break;
            case 2:
                switch(code.getP()) {
                    case 0:
                        instruction.destination.reg = reg::BC;
                        break;
                    case 1:
                        instruction.destination.reg = reg::DE;
                        break;
                    case 2:
                        instruction.destination.reg = reg::HL;
                        instruction.LD_subtype = LoadSubtype::LD_INC;
                        break;
                    case 3:
                        instruction.destination.reg = reg::HL;
                        instruction.LD_subtype = LoadSubtype::LD_DEC;
                        break;
                }

                instruction.source.source = arg_src::Register;
                instruction.source.type = arg_t::Unsigned8;
                instruction.source.reg = reg::A;
                instruction.destination.source = arg_src::Indirect;
                instruction.destination.type = arg_t::Unsigned8;
                if(code.getQ()) {
                    std::swap(instruction.destination, instruction.source);
                }
                break;
            case 6:
                instruction.source.source = arg_src::Immediate;
                instruction.source.type = arg_t::Unsigned8;
                                
                setRegisterInfo(code.getY(), instruction.destination);
                break;
        }

        if(!instruction.LD_subtype) {
            instruction.LD_subtype = LoadSubtype::Typical;
        }
    }

    void decodeJR(opcode code, UnprefixedInstruction& instruction) {
        if(code.getY() != 3) {
            instruction.condition = g_conditions_[code.getY() - 4];
        }
        instruction.source.source = arg_src::Immediate;
        instruction.source.type = arg_t::Signed8;
    }

    void decodeJP(opcode code, UnprefixedInstruction& instruction) {
        switch(code.getZ()) {
            case 1:
                instruction.source.source = arg_src::Register;
                instruction.source.type = arg_t::Unsigned16;
                instruction.source.reg = reg::HL;
                break;
            case 2:
                instruction.condition = g_conditions_[code.getY()];
                [[fallthrough]];
            case 3:
                instruction.source.source = arg_src::Immediate;
                instruction.source.type = arg_t::Unsigned16;
                break;
        }
    }

    void decodeINC_DEC(opcode code, UnprefixedInstruction& instruction) {
        switch(code.getZ()) {
            case 3:
                instruction.source.source = arg_src::Register;
                instruction.source.type = arg_t::Unsigned16;
                instruction.source.reg = g_word_registers_SP_[code.getP()];
                break;
            case 4:
            case 5:
                setRegisterInfo(code.getY(), instruction.source);
                break;
        }
        instruction.destination = instruction.source;
    }

    PrefixedInstruction decodePrefixed(opcode code) {
        PrefixedInstruction result;

        switch (code.getX()) {
            case 0: 
                result.type = g_bit_operations_[code.getY()];
                break;
            case 1: 
                result.type = pref_type::BIT;
                result.bit = code.getY();
                break;
            case 2: 
                result.type = pref_type::RES;
                result.bit = code.getY();
                break;
            case 3: 
                result.type = pref_type::SET;
                result.bit = code.getY();
                break;
        }
        result.target = g_byte_registers_[code.getZ()];

        return result;
    }

    void setRegisterInfo(uint8_t registerIndex, ArgumentInfo& registerInfo) {
        registerInfo.reg = g_byte_registers_[registerIndex];
        registerInfo.type = arg_t::Unsigned8;

        if(registerInfo.reg != reg::HL) {
            registerInfo.source = arg_src::Register;
        } else {
            registerInfo.source = arg_src::Indirect;
        }
    }

    void setALUInfo(opcode code, UnprefixedInstruction& instruction, bool hasImmediate) {
        instruction.type = g_ALU_[code.getY()];
        instruction.destination.source = arg_src::Register;
        instruction.destination.type = arg_t::Unsigned8;
        instruction.destination.reg = reg::A;
                    
        if(hasImmediate) {
            instruction.source.source = arg_src::Immediate;
            instruction.source.type = arg_t::Unsigned8;
        } else {
            setRegisterInfo(code.getZ(), instruction.source);
        }
    }

}

