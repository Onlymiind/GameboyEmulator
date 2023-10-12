#include "gb/cpu/Decoder.h"
#include "gb/cpu/Operation.h"

#include <utility>

namespace gb::decoding {

    using type = InstructionType;
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

    constexpr std::array<InstructionType, 8> g_ALU_ = {
        InstructionType::ADD, InstructionType::ADC, InstructionType::SUB, InstructionType::SBC,
        InstructionType::AND, InstructionType::XOR, InstructionType::OR,  InstructionType::CP
    };

    constexpr std::array<InstructionType, 8> g_bit_operations_ = {
        InstructionType::RLC,  InstructionType::RRC,
        InstructionType::RL,   InstructionType::RR,
        InstructionType::SLA,  InstructionType::SRA,
        InstructionType::SWAP, InstructionType::SRL
    };

    static const std::unordered_map<uint8_t, InstructionType, column_hash, collideable_equal> g_columns_ = {
        { 0x01, InstructionType::LD }, { 0x02, InstructionType::LD }, { 0x03, InstructionType::INC },
        { 0x04, InstructionType::INC }, { 0x05, InstructionType::DEC }, { 0x06, InstructionType::LD },
        { 0x09, InstructionType::ADD }, { 0x0A, InstructionType::LD }, { 0x0B, InstructionType::DEC },
        { 0x0C, InstructionType::INC }, { 0x0D, InstructionType::DEC }, { 0x0E, InstructionType::LD },
        { 0xC1, InstructionType::POP }, { 0xC5, InstructionType::PUSH }, { 0xC7, InstructionType::RST },
        { 0xCF, InstructionType::RST }
    };

    static const std::unordered_map<uint8_t, InstructionType> g_random_instructions_ = {
        { 0x00, InstructionType::NOP }, { 0x07, InstructionType::RLCA }, { 0x08, InstructionType::LD }, { 0x0F, InstructionType::RRCA },
        
        { 0x10, InstructionType::STOP }, { 0x17, InstructionType::RLA }, { 0x18, InstructionType::JR }, { 0x1F, InstructionType::RRA },
        
        { 0x20, InstructionType::JR }, { 0x27, InstructionType::DAA }, { 0x28, InstructionType::JR }, { 0x2F, InstructionType::CPL },
        
        { 0x30, InstructionType::JR }, { 0x37, InstructionType::SCF }, { 0x38, InstructionType::JR }, { 0x3F, InstructionType::CCF },
        { 0xC0, InstructionType::RET }, { 0xC2, InstructionType::JP }, { 0xC3, InstructionType::JP }, { 0xC4, InstructionType::CALL },
        { 0xC8, InstructionType::RET }, { 0xC9, InstructionType::RET }, { 0xCA, InstructionType::JP }, { 0xCC, InstructionType::CALL },
        { 0xCD, InstructionType::CALL }, 
        
        { 0xD0, InstructionType::RET }, { 0xD2, InstructionType::JP }, { 0xD4, InstructionType::CALL }, { 0xD8, InstructionType::RET },
        { 0xD9, InstructionType::RETI }, {0xDA, InstructionType::JP }, { 0xDC, InstructionType::CALL }, 
        
        { 0xE0, InstructionType::LD }, { 0xE2, InstructionType::LD }, { 0xE8, InstructionType::ADD }, { 0xE9, InstructionType::JP }, 
        { 0xEA, InstructionType::LD },
        
        { 0xF0, InstructionType::LD }, { 0xF2, InstructionType::LD }, { 0xF3, InstructionType::DI }, { 0xF8, InstructionType::LD }, 
        { 0xF9, InstructionType::LD }, { 0xFA, InstructionType::LD }, { 0xFB, InstructionType::EI },
    };

    //Some LD instructions are a pain to decode, so it is done with this lookup table
    static const std::unordered_map<uint8_t, UnprefixedInstruction> g_random_LD_ = {
        {0x08, {
                {}, //Reset vector
                LoadSubtype::LD_SP, //LD Subtype
                {}, //Condition
                {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP}, //Source
                {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None}, //Destination
                InstructionType::LD //Instruction type
            }},
        {0xE0, {
                {},
                LoadSubtype::LD_IO,
                {},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None},
                InstructionType::LD
            }}, 
        {0xF0, {
                {},
                LoadSubtype::LD_IO,
                {},
                {ArgumentSource::Immediate, ArgumentType::Unsigned8, Registers::None},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                InstructionType::LD
            }},
        {0xE2, {
                {},
                LoadSubtype::LD_IO,
                {},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                {ArgumentSource::Indirect, ArgumentType::Unsigned8, Registers::C},
                InstructionType::LD
            }},
        {0xF2, {
                {},
                LoadSubtype::LD_IO,
                {},
                {ArgumentSource::Indirect, ArgumentType::Unsigned8, Registers::C},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                InstructionType::LD
            }},
        {0xF8, {
                {},
                LoadSubtype::LD_Offset_SP,
                {},
                {ArgumentSource::Immediate, ArgumentType::Signed8, Registers::None},
                {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::HL},
                InstructionType::LD
            }},
        {0xF9, {
                {},
                LoadSubtype::Typical,
                {},
                {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::HL},
                {ArgumentSource::Register, ArgumentType::Unsigned16, Registers::SP},
                InstructionType::LD
            }},
        {0xEA, {
                {},
                LoadSubtype::Typical,
                {},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None},
                InstructionType::LD
            }},
        {0xFA, {
                {},
                LoadSubtype::Typical,
                {},
                {ArgumentSource::IndirectImmediate, ArgumentType::Unsigned16, Registers::None},
                {ArgumentSource::Register, ArgumentType::Unsigned8, Registers::A},
                InstructionType::LD
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
            auto it = g_columns_.find(code.code);
            if(it != g_columns_.end()) {
                result.type = it->second;
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
                result.type = type::BIT;
                result.bit = code.getY();
                break;
            case 2: 
                result.type = type::RES;
                result.bit = code.getY();
                break;
            case 3: 
                result.type = type::SET;
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

