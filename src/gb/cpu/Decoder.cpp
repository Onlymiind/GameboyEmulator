#include "gb/cpu/Decoder.h"
#include "gb/cpu/CPU.h"
#include "gb/cpu/Operation.h"

#include <cstdint>
#include <stdexcept>
#include <utility>

// TODO: this is a mess
namespace gb::cpu {

    using Type = InstructionType;
    using ArgSrc = ArgumentSource;
    using ArgType = ArgumentType;
    using Reg = Registers;

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

    static void setRegisterInfo(uint8_t register_index, ArgumentInfo &register_info);
    static void setALUInfo(Opcode code, DecodedInstruction &instruction, bool has_immediate);
    static DecodedInstruction decodeIrregularInstruction(Opcode code);
    static DecodedInstruction decodeColumn(Opcode code);

    consteval uint8_t getColumnID(uint8_t quarter, uint8_t column) {
        return (quarter << 6) | column;
    }

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
            result = decodeIrregularInstruction(code);
        }

        return result;
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

        setRegisterInfo(code.getZ(), result.arg());

        return result;
    }

    void setRegisterInfo(uint8_t register_index, ArgumentInfo &register_info) {
        register_info.reg = g_byte_registers[register_index];
        register_info.src = register_info.reg == Registers::HL ? ArgumentSource::INDIRECT
                                                               : ArgumentSource::REGISTER;
    }

    void setALUInfo(Opcode code, DecodedInstruction &instruction, bool has_immediate) {
        instruction.type = g_alu[code.getY()];
        instruction.dst.src = ArgSrc::REGISTER;
        instruction.dst.reg = Reg::A;

        if (has_immediate) {
            instruction.src.src = ArgSrc::IMMEDIATE_U8;
        } else {
            setRegisterInfo(code.getZ(), instruction.src);
        }
    }

    DecodedInstruction decodeIrregularInstruction(Opcode code) {

        if (auto instr = decodeColumn(code); instr.type != Type::NONE) {
            return instr;
        }

        switch (code.code) {
        case 0x00:
            return DecodedInstruction{.type = Type::NOP};
        case 0x07:
            return DecodedInstruction{.type = Type::RLCA};
        case 0x08:
            return DecodedInstruction{
                .ld_subtype = LoadSubtype::LD_SP,
                .src = ArgumentInfo{ArgumentSource::DOUBLE_REGISTER, Registers::SP},
                .dst = ArgumentInfo{ArgumentSource::IMMEDIATE_U16, Registers::NONE},
                .type = InstructionType::LD};
        case 0x0F:
            return DecodedInstruction{.type = Type::RRCA};
        case 0x10:
            return DecodedInstruction{.type = Type::STOP};
        case 0x17:
            return DecodedInstruction{.type = Type::RLA};
        case 0x1F:
            return DecodedInstruction{.type = Type::RRA};
        case 0x27:
            return DecodedInstruction{.type = Type::DAA};
        case 0x2F:
            return DecodedInstruction{.type = Type::CPL};
        case 0x37:
            return DecodedInstruction{.type = Type::SCF};
        case 0x3F:
            return DecodedInstruction{.type = Type::CCF};
        case 0x18: // JRs
            return DecodedInstruction{.src = ArgumentInfo{ArgSrc::IMMEDIATE_S8}, .type = Type::JR};
        case 0x20:
            [[fallthrough]];
        case 0x28:
            [[fallthrough]];
        case 0x30:
            [[fallthrough]];
        case 0x38:
            return DecodedInstruction{.condition = g_conditions[code.getY() - 4],
                                      .src = ArgumentInfo{ArgSrc::IMMEDIATE_S8},
                                      .type = Type::JR};
        case 0xC0: // RETs
            [[fallthrough]];
        case 0xC8:
            [[fallthrough]];
        case 0xD0:
            [[fallthrough]];
        case 0xD8:
            return DecodedInstruction{.condition = g_conditions[code.getY()], .type = Type::RET};
        case 0xC9:
            return DecodedInstruction{.type = Type::RET};
        case 0xD9:
            return DecodedInstruction{.type = Type::RETI};
        case 0xC2: // JPs
            [[fallthrough]];
        case 0xD2:
            [[fallthrough]];
        case 0xCA:
            [[fallthrough]];
        case 0xDA:
            return DecodedInstruction{.condition = g_conditions[code.getY()],
                                      .src = ArgumentInfo{ArgSrc::IMMEDIATE_U16},
                                      .type = Type::JP};
        case 0xE9:
            return DecodedInstruction{.src = ArgumentInfo{ArgSrc::DOUBLE_REGISTER, Reg::HL},
                                      .type = Type::JP};
        case 0xC3:
            return DecodedInstruction{.src = ArgumentInfo{ArgSrc::IMMEDIATE_U16}, .type = Type::JP};
        case 0xC4: // CALLs
            [[fallthrough]];
        case 0xD4:
            [[fallthrough]];
        case 0xCC:
            [[fallthrough]];
        case 0xDC: {
            DecodedInstruction instr{.condition = g_conditions[code.getY()], .type = Type::CALL};
            instr.arg().src = ArgumentSource::IMMEDIATE_U16;
            return instr;
        }
        case 0xCD: {
            DecodedInstruction instr{.type = Type::CALL};
            instr.arg().src = ArgumentSource::IMMEDIATE_U16;
            return instr;
        }
        case 0xE8: // ADD SP, e
            return DecodedInstruction{.src = ArgumentInfo{ArgSrc::IMMEDIATE_S8},
                                      .dst = ArgumentInfo{ArgSrc::DOUBLE_REGISTER, Reg::SP},
                                      .type = Type::ADD};
        case 0xE0:
            return DecodedInstruction{
                .ld_subtype = LoadSubtype::LD_IO,
                .src = ArgumentInfo{ArgumentSource::REGISTER, Registers::A},
                .dst = ArgumentInfo{ArgumentSource::IMMEDIATE_U8, Registers::NONE},
                .type = InstructionType::LD};
        case 0xE2:
            return DecodedInstruction{.ld_subtype = LoadSubtype::LD_IO,
                                      .src = ArgumentInfo{ArgumentSource::REGISTER, Registers::A},
                                      .dst = ArgumentInfo{ArgumentSource::INDIRECT, Registers::C},
                                      .type = InstructionType::LD};
        case 0xEA:
            return DecodedInstruction{
                .ld_subtype = LoadSubtype::TYPICAL,
                .src = ArgumentInfo{ArgumentSource::REGISTER, Registers::A},
                .dst = ArgumentInfo{ArgumentSource::IMMEDIATE_U16, Registers::NONE},
                .type = InstructionType::LD};
        case 0xF0:
            return DecodedInstruction{
                .ld_subtype = LoadSubtype::LD_IO,
                .src = ArgumentInfo{ArgumentSource::IMMEDIATE_U8, Registers::NONE},
                .dst = ArgumentInfo{ArgumentSource::REGISTER, Registers::A},
                .type = InstructionType::LD};
        case 0xF2:
            return DecodedInstruction{.ld_subtype = LoadSubtype::LD_IO,
                                      .src = ArgumentInfo{ArgumentSource::INDIRECT, Registers::C},
                                      .dst = ArgumentInfo{ArgumentSource::REGISTER, Registers::A},
                                      .type = InstructionType::LD};
        case 0xF3:
            return DecodedInstruction{.type = Type::DI};
        case 0xF8:
            return DecodedInstruction{
                .ld_subtype = LoadSubtype::LD_OFFSET_SP,
                .src = ArgumentInfo{ArgumentSource::IMMEDIATE_S8, Registers::NONE},
                .dst = ArgumentInfo{ArgumentSource::DOUBLE_REGISTER, Registers::HL},
                .type = InstructionType::LD};
        case 0xF9:
            return DecodedInstruction{
                .ld_subtype = LoadSubtype::TYPICAL,
                .src = ArgumentInfo{ArgumentSource::DOUBLE_REGISTER, Registers::HL},
                .dst = ArgumentInfo{ArgumentSource::DOUBLE_REGISTER, Registers::SP},
                .type = InstructionType::LD};
        case 0xFA:
            return DecodedInstruction{
                .ld_subtype = LoadSubtype::TYPICAL,
                .src = ArgumentInfo{ArgumentSource::IMMEDIATE_U16, Registers::NONE},
                .dst = ArgumentInfo{ArgumentSource::REGISTER, Registers::A},
                .type = InstructionType::LD};
        case 0xFB:
            return DecodedInstruction{.type = Type::EI};
        default:
            throw std::invalid_argument("invalid instruction");
        }
    }

    DecodedInstruction decodeColumn(Opcode code) {
        DecodedInstruction instr{};
        switch (code.getColumn()) {
        case getColumnID(0, 1):
            instr.ld_subtype = LoadSubtype::TYPICAL;
            instr.src.src = ArgSrc::IMMEDIATE_U16, instr.dst.src = ArgSrc::DOUBLE_REGISTER;
            instr.dst.reg = g_word_registers_sp[code.getP()];
            instr.type = Type::LD;
            break;
        case getColumnID(0, 3):
            instr.src.src = ArgSrc::DOUBLE_REGISTER;
            instr.src.reg = g_word_registers_sp[code.getP()];
            instr.type = Type::INC;
            break;
        case getColumnID(0, 0xB):
            instr.src.src = ArgSrc::DOUBLE_REGISTER;
            instr.src.reg = g_word_registers_sp[code.getP()];
            instr.type = Type::DEC;
            break;
        case getColumnID(0, 4):
            [[fallthrough]];
        case getColumnID(0, 0xC):
            instr.type = Type::INC;
            setRegisterInfo(code.getY(), instr.src);
            break;
        case getColumnID(0, 5):
            [[fallthrough]];
        case getColumnID(0, 0xD):
            instr.type = Type::DEC;
            setRegisterInfo(code.getY(), instr.src);
            break;
        case getColumnID(0, 6):
            instr.src.src = ArgSrc::IMMEDIATE_U8;
            instr.type = Type::LD;
            instr.ld_subtype = LoadSubtype::TYPICAL;
            setRegisterInfo(code.getY(), instr.dst);
            break;
        case getColumnID(0, 2):
            [[fallthrough]];
        case getColumnID(0, 0xA):
            switch (code.getP()) {
            case 0:
                instr.dst.reg = Reg::BC;
                instr.ld_subtype = LoadSubtype::TYPICAL;
                break;
            case 1:
                instr.dst.reg = Reg::DE;
                instr.ld_subtype = LoadSubtype::TYPICAL;
                break;
            case 2:
                instr.dst.reg = Reg::HL;
                instr.ld_subtype = LoadSubtype::LD_INC;
                break;
            case 3:
                instr.dst.reg = Reg::HL;
                instr.ld_subtype = LoadSubtype::LD_DEC;
                break;
            }

            instr.src.src = ArgSrc::REGISTER;
            instr.src.reg = Reg::A;
            instr.dst.src = ArgSrc::INDIRECT;
            instr.type = Type::LD;
            if (code.getQ()) {
                std::swap(instr.dst, instr.src);
            }
            break;
        case getColumnID(0, 9):
            instr.dst.src = ArgSrc::DOUBLE_REGISTER;
            instr.dst.reg = Reg::HL;
            instr.src.src = ArgSrc::DOUBLE_REGISTER;
            instr.src.reg = g_word_registers_sp[code.getP()];
            instr.type = Type::ADD;
            break;
        case getColumnID(0, 0xE):
            instr.src.src = ArgSrc::IMMEDIATE_U8;
            instr.type = Type::LD;
            instr.ld_subtype = LoadSubtype::TYPICAL;
            setRegisterInfo(code.getY(), instr.dst);
            break;
        case getColumnID(3, 1):
            instr.dst.src = ArgSrc::DOUBLE_REGISTER;
            instr.dst.reg = g_word_registers_af[code.getP()];
            instr.type = Type::POP;
            break;
        case getColumnID(3, 5):
            instr.src.src = ArgSrc::DOUBLE_REGISTER;
            instr.src.reg = g_word_registers_af[code.getP()];
            instr.type = Type::PUSH;
            break;
        case getColumnID(3, 7): // RST
            [[fallthrough]];
        case getColumnID(3, 0xF):
            instr.reset_vector = code.getY() * 8;
            instr.type = Type::RST;
            break;
        }

        return instr;
    }

} // namespace gb::cpu
