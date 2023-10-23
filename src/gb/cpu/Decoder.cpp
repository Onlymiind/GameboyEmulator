#include "gb/cpu/Decoder.h"
#include "gb/cpu/Operation.h"

#include <utility>

// TODO: this is a mess
namespace gb::cpu {

    using Type = InstructionType;
    using ArgSrc = ArgumentSource;
    using ArgType = ArgumentType;
    using Reg = Registers;

    struct ColumnHash {
        inline size_t operator()(uint8_t value) const { return value & 0b1100'1111; }
    };
    struct CollideableEqual {
        inline bool operator()(const uint8_t &lhs, const uint8_t &rhs) const {
            return ColumnHash{}(lhs) == ColumnHash{}(rhs);
        }
    };

    constexpr std::array<Registers, 8> g_byte_registers = {
        Registers::B, Registers::C, Registers::D,  Registers::E,
        Registers::H, Registers::L, Registers::HL, Registers::A};

    // Register pair lookup with SP
    constexpr std::array<Registers, 4> g_word_registers_sp = {Registers::BC, Registers::DE,
                                                              Registers::HL, Registers::SP};

    // Register pair lookup with AF
    constexpr std::array<Registers, 4> g_word_registers_af = {Registers::BC, Registers::DE,
                                                              Registers::HL, Registers::AF};

    // Conditions lookup
    constexpr std::array<Conditions, 4> g_conditions = {Conditions::NOT_ZERO, Conditions::ZERO,
                                                        Conditions::NOT_CARRY, Conditions::CARRY};

    constexpr std::array<InstructionType, 8> g_alu = {
        InstructionType::ADD, InstructionType::ADC, InstructionType::SUB, InstructionType::SBC,
        InstructionType::AND, InstructionType::XOR, InstructionType::OR,  InstructionType::CP};

    constexpr std::array<InstructionType, 8> g_bit_operations = {
        InstructionType::RLC, InstructionType::RRC, InstructionType::RL,   InstructionType::RR,
        InstructionType::SLA, InstructionType::SRA, InstructionType::SWAP, InstructionType::SRL};

    static const std::unordered_map<uint8_t, InstructionType, ColumnHash, CollideableEqual>
        g_columns = {{0x01, InstructionType::LD},  {0x02, InstructionType::LD},
                     {0x03, InstructionType::INC}, {0x04, InstructionType::INC},
                     {0x05, InstructionType::DEC}, {0x06, InstructionType::LD},
                     {0x09, InstructionType::ADD}, {0x0A, InstructionType::LD},
                     {0x0B, InstructionType::DEC}, {0x0C, InstructionType::INC},
                     {0x0D, InstructionType::DEC}, {0x0E, InstructionType::LD},
                     {0xC1, InstructionType::POP}, {0xC5, InstructionType::PUSH},
                     {0xC7, InstructionType::RST}, {0xCF, InstructionType::RST}};

    static const std::unordered_map<uint8_t, InstructionType> g_random_instructions = {
        {0x00, InstructionType::NOP},  {0x07, InstructionType::RLCA}, {0x08, InstructionType::LD},
        {0x0F, InstructionType::RRCA},

        {0x10, InstructionType::STOP}, {0x17, InstructionType::RLA},  {0x18, InstructionType::JR},
        {0x1F, InstructionType::RRA},

        {0x20, InstructionType::JR},   {0x27, InstructionType::DAA},  {0x28, InstructionType::JR},
        {0x2F, InstructionType::CPL},

        {0x30, InstructionType::JR},   {0x37, InstructionType::SCF},  {0x38, InstructionType::JR},
        {0x3F, InstructionType::CCF},  {0xC0, InstructionType::RET},  {0xC2, InstructionType::JP},
        {0xC3, InstructionType::JP},   {0xC4, InstructionType::CALL}, {0xC8, InstructionType::RET},
        {0xC9, InstructionType::RET},  {0xCA, InstructionType::JP},   {0xCC, InstructionType::CALL},
        {0xCD, InstructionType::CALL},

        {0xD0, InstructionType::RET},  {0xD2, InstructionType::JP},   {0xD4, InstructionType::CALL},
        {0xD8, InstructionType::RET},  {0xD9, InstructionType::RETI}, {0xDA, InstructionType::JP},
        {0xDC, InstructionType::CALL},

        {0xE0, InstructionType::LD},   {0xE2, InstructionType::LD},   {0xE8, InstructionType::ADD},
        {0xE9, InstructionType::JP},   {0xEA, InstructionType::LD},

        {0xF0, InstructionType::LD},   {0xF2, InstructionType::LD},   {0xF3, InstructionType::DI},
        {0xF8, InstructionType::LD},   {0xF9, InstructionType::LD},   {0xFA, InstructionType::LD},
        {0xFB, InstructionType::EI},
    };

    // Some LD instructions are a pain to decode, so it is done with this lookup
    // table
    static const std::unordered_map<uint8_t, DecodedInstruction> g_random_ld = {
        {0x08,
         {
             {},                                                                   // Reset vector
             LoadSubtype::LD_SP,                                                   // LD Subtype
             {},                                                                   // Condition
             {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_16, Registers::SP}, // Source
             {ArgumentSource::INDIRECT_IMMEDIATE, ArgumentType::UNSIGNED_16,
              Registers::NONE},  // Destination
             InstructionType::LD // Instruction type
         }},
        {0xE0,
         {{},
          LoadSubtype::LD_IO,
          {},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_8, Registers::A},
          {ArgumentSource::IMMEDIATE, ArgumentType::UNSIGNED_8, Registers::NONE},
          InstructionType::LD}},
        {0xF0,
         {{},
          LoadSubtype::LD_IO,
          {},
          {ArgumentSource::IMMEDIATE, ArgumentType::UNSIGNED_8, Registers::NONE},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_8, Registers::A},
          InstructionType::LD}},
        {0xE2,
         {{},
          LoadSubtype::LD_IO,
          {},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_8, Registers::A},
          {ArgumentSource::INDIRECT, ArgumentType::UNSIGNED_8, Registers::C},
          InstructionType::LD}},
        {0xF2,
         {{},
          LoadSubtype::LD_IO,
          {},
          {ArgumentSource::INDIRECT, ArgumentType::UNSIGNED_8, Registers::C},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_8, Registers::A},
          InstructionType::LD}},
        {0xF8,
         {{},
          LoadSubtype::LD_OFFSET_SP,
          {},
          {ArgumentSource::IMMEDIATE, ArgumentType::SIGNED_8, Registers::NONE},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_16, Registers::HL},
          InstructionType::LD}},
        {0xF9,
         {{},
          LoadSubtype::TYPICAL,
          {},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_16, Registers::HL},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_16, Registers::SP},
          InstructionType::LD}},
        {0xEA,
         {{},
          LoadSubtype::TYPICAL,
          {},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_8, Registers::A},
          {ArgumentSource::INDIRECT_IMMEDIATE, ArgumentType::UNSIGNED_16, Registers::NONE},
          InstructionType::LD}},
        {0xFA,
         {{},
          LoadSubtype::TYPICAL,
          {},
          {ArgumentSource::INDIRECT_IMMEDIATE, ArgumentType::UNSIGNED_16, Registers::NONE},
          {ArgumentSource::REGISTER, ArgumentType::UNSIGNED_8, Registers::A},
          InstructionType::LD}}};

    void setRegisterInfo(uint8_t register_index, ArgumentInfo &register_info);
    void setALUInfo(Opcode code, DecodedInstruction &instruction, bool has_immediate);
    void decodeRandomInstructions(Opcode code, DecodedInstruction &instruction);
    void decodeADD(Opcode code, DecodedInstruction &instruction);
    void decodeLD(Opcode code, DecodedInstruction &instruction);
    void decodeJR(Opcode code, DecodedInstruction &instruction);
    void decodeJP(Opcode code, DecodedInstruction &instruction);
    void decodeINC_DEC(Opcode code, DecodedInstruction &instruction);

    DecodedInstruction decodeUnprefixed(Opcode code) {
        DecodedInstruction result;

        switch (code.getX()) {
        case 1:
            if (code.getZ() == 6 && code.getY() == 6) {
                result.type = Type::HALT;
            } else {
                result.type = Type::LD;
                result.ld_subtype = LoadSubtype::TYPICAL;
                setRegisterInfo(code.getY(), result.dst);
                setRegisterInfo(code.getZ(), result.src);
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

        if (result.type == Type::NONE) {
            bool is_random_ld = false;
            auto it = g_columns.find(code.code);
            if (it != g_columns.end()) {
                result.type = it->second;
            } else {
                result.type = g_random_instructions.at(code.code);
                if (result.type == Type::LD) {
                    is_random_ld = true;
                }
            }
            if (!is_random_ld) {
                decodeRandomInstructions(code, result);
            } else {
                result = g_random_ld.at(code.code);
            }
        }

        return result;
    }

    void decodeRandomInstructions(Opcode code, DecodedInstruction &instruction) {
        switch (instruction.type) {
        case Type::NOP:
        case Type::STOP:
        case Type::SCF:
        case Type::CCF:
        case Type::CPL:
        case Type::DAA:
        case Type::DI:
        case Type::EI:
        case Type::RETI:
        case Type::RLCA:
        case Type::RRCA:
        case Type::RLA:
        case Type::RRA:
            return;
        case Type::RST:
            instruction.reset_vector = code.getY() * 8;
            return;
        case Type::POP:
            instruction.dst.reg = g_word_registers_af[code.getP()];
            return;
        case Type::PUSH:
            instruction.src.reg = g_word_registers_af[code.getP()];
            return;
        case Type::RET:
            if (code.getZ() == 0) {
                instruction.condition = g_conditions[code.getY()];
            }
            return;
        case Type::ADD:
            decodeADD(code, instruction);
            return;
        case Type::JR:
            decodeJR(code, instruction);
            return;
        case Type::JP:
            decodeJP(code, instruction);
            return;
        case Type::CALL:
            if (code.getZ() == 4) {
                instruction.condition = g_conditions[code.getY()];
            }
            instruction.arg().src = ArgumentSource::IMMEDIATE;
            instruction.arg().type = ArgumentType::UNSIGNED_16;
            return;
        case Type::INC:
        case Type::DEC:
            decodeINC_DEC(code, instruction);
            return;
        case Type::LD:
            decodeLD(code, instruction);
            return;
        default:
            return;
        }
    }

    void decodeADD(Opcode code, DecodedInstruction &instruction) {
        switch (code.getZ()) {
        case 0:
            instruction.dst.src = ArgSrc::REGISTER;
            instruction.dst.type = ArgType::UNSIGNED_16;
            instruction.dst.reg = Reg::SP;
            instruction.src.src = ArgSrc::IMMEDIATE;
            instruction.src.type = ArgType::SIGNED_8;
            break;
        case 1:
            instruction.dst.src = ArgSrc::REGISTER;
            instruction.dst.type = ArgType::UNSIGNED_16;
            instruction.dst.reg = Reg::HL;
            instruction.src.src = ArgSrc::REGISTER;
            instruction.src.type = ArgType::UNSIGNED_16;
            instruction.src.reg = g_word_registers_sp[code.getP()];
            break;
        }
    }

    void decodeLD(Opcode code, DecodedInstruction &instruction) {
        switch (code.getZ()) {
        case 1:
            instruction.dst.reg = g_word_registers_sp[code.getP()];
            instruction.dst.src = ArgSrc::REGISTER;
            instruction.dst.type = ArgType::UNSIGNED_16;
            instruction.src.src = ArgSrc::IMMEDIATE;
            instruction.src.type = ArgType::UNSIGNED_16;
            break;
        case 2:
            switch (code.getP()) {
            case 0:
                instruction.dst.reg = Reg::BC;
                break;
            case 1:
                instruction.dst.reg = Reg::DE;
                break;
            case 2:
                instruction.dst.reg = Reg::HL;
                instruction.ld_subtype = LoadSubtype::LD_INC;
                break;
            case 3:
                instruction.dst.reg = Reg::HL;
                instruction.ld_subtype = LoadSubtype::LD_DEC;
                break;
            }

            instruction.src.src = ArgSrc::REGISTER;
            instruction.src.type = ArgType::UNSIGNED_8;
            instruction.src.reg = Reg::A;
            instruction.dst.src = ArgSrc::INDIRECT;
            instruction.dst.type = ArgType::UNSIGNED_8;
            if (code.getQ()) {
                std::swap(instruction.dst, instruction.src);
            }
            break;
        case 6:
            instruction.src.src = ArgSrc::IMMEDIATE;
            instruction.src.type = ArgType::UNSIGNED_8;

            setRegisterInfo(code.getY(), instruction.dst);
            break;
        }

        if (!instruction.ld_subtype) {
            instruction.ld_subtype = LoadSubtype::TYPICAL;
        }
    }

    void decodeJR(Opcode code, DecodedInstruction &instruction) {
        if (code.getY() != 3) {
            instruction.condition = g_conditions[code.getY() - 4];
        }
        instruction.src.src = ArgSrc::IMMEDIATE;
        instruction.src.type = ArgType::SIGNED_8;
    }

    void decodeJP(Opcode code, DecodedInstruction &instruction) {
        switch (code.getZ()) {
        case 1:
            instruction.src.src = ArgSrc::REGISTER;
            instruction.src.type = ArgType::UNSIGNED_16;
            instruction.src.reg = Reg::HL;
            break;
        case 2:
            instruction.condition = g_conditions[code.getY()];
            [[fallthrough]];
        case 3:
            instruction.src.src = ArgSrc::IMMEDIATE;
            instruction.src.type = ArgType::UNSIGNED_16;
            break;
        }
    }

    void decodeINC_DEC(Opcode code, DecodedInstruction &instruction) {
        switch (code.getZ()) {
        case 3:
            instruction.src.src = ArgSrc::REGISTER;
            instruction.src.type = ArgType::UNSIGNED_16;
            instruction.src.reg = g_word_registers_sp[code.getP()];
            break;
        case 4:
        case 5:
            setRegisterInfo(code.getY(), instruction.src);
            break;
        }
        instruction.dst = instruction.src;
    }

    DecodedInstruction decodePrefixed(Opcode code) {
        DecodedInstruction result;

        switch (code.getX()) {
        case 0:
            result.type = g_bit_operations[code.getY()];
            break;
        case 1:
            result.type = Type::BIT;
            result.bit = code.getY();
            break;
        case 2:
            result.type = Type::RES;
            result.bit = code.getY();
            break;
        case 3:
            result.type = Type::SET;
            result.bit = code.getY();
            break;
        }

        auto &arg = result.arg();
        arg.reg = g_byte_registers[code.getZ()];
        arg.type = ArgumentType::UNSIGNED_8;
        arg.src = arg.reg == Registers::HL ? ArgumentSource::INDIRECT : ArgumentSource::REGISTER;

        return result;
    }

    void setRegisterInfo(uint8_t register_index, ArgumentInfo &register_info) {
        register_info.reg = g_byte_registers[register_index];
        register_info.type = ArgType::UNSIGNED_8;

        if (register_info.reg != Reg::HL) {
            register_info.src = ArgSrc::REGISTER;
        } else {
            register_info.src = ArgSrc::INDIRECT;
        }
    }

    void setALUInfo(Opcode code, DecodedInstruction &instruction, bool has_immediate) {
        instruction.type = g_alu[code.getY()];
        instruction.dst.src = ArgSrc::REGISTER;
        instruction.dst.type = ArgType::UNSIGNED_8;
        instruction.dst.reg = Reg::A;

        if (has_immediate) {
            instruction.src.src = ArgSrc::IMMEDIATE;
            instruction.src.type = ArgType::UNSIGNED_8;
        } else {
            setRegisterInfo(code.getZ(), instruction.src);
        }
    }

} // namespace gb::cpu
