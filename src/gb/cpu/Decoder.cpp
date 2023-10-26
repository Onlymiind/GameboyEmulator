#include "gb/cpu/Decoder.h"
#include "gb/cpu/CPU.h"
#include "gb/cpu/Operation.h"

#include <array>
#include <cstdint>
#include <stdexcept>
#include <utility>

// TODO: this is a mess
namespace gb::cpu {

    using Type = InstructionType;
    using ArgSrc = ArgumentSource;
    using ArgType = ArgumentType;
    using Reg = Registers;

    static constexpr void setRegisterInfo(uint8_t register_index, ArgumentInfo &register_info);
    static constexpr void setALUInfo(Opcode code, DecodedInstruction &instruction,
                                     bool has_immediate);

    consteval DecodedInstruction dec(Registers reg, bool double_reg = false) {
        DecodedInstruction instr{.type = Type::DEC};
        instr.arg().reg = reg;
        if (double_reg) {
            instr.arg().src = ArgSrc::DOUBLE_REGISTER;
        } else {
            instr.arg().src = reg == Registers::HL ? ArgSrc::INDIRECT : ArgSrc::REGISTER;
        }

        return instr;
    }

    consteval DecodedInstruction inc(Registers reg, bool double_reg = false) {
        DecodedInstruction instr{.type = Type::INC};
        instr.arg().reg = reg;
        if (double_reg) {
            instr.arg().src = ArgSrc::DOUBLE_REGISTER;
        } else {
            instr.arg().src = reg == Registers::HL ? ArgSrc::INDIRECT : ArgSrc::REGISTER;
        }

        return instr;
    }

    consteval DecodedInstruction rst(uint16_t address) {
        return DecodedInstruction{.reset_vector = address, .type = Type::RST};
    }

    consteval DecodedInstruction jp(std::optional<Conditions> condition = {}, bool jp_hl = false) {
        DecodedInstruction instr{.type = Type::JP};
        if (jp_hl) {
            instr.arg().src = ArgSrc::DOUBLE_REGISTER;
            instr.arg().reg = Registers::HL;
        } else {
            instr.arg().src = ArgSrc::IMMEDIATE_U16;
            instr.condition = condition;
        }
        return instr;
    }

    consteval DecodedInstruction call(std::optional<Conditions> condition = {}) {
        DecodedInstruction instr{.type = Type::CALL};
        instr.arg().src = ArgSrc::IMMEDIATE_U16;
        instr.condition = condition;
        return instr;
    }

    consteval DecodedInstruction jr(std::optional<Conditions> condition = {}) {
        DecodedInstruction instr{.type = Type::JR};
        instr.arg().src = ArgSrc::IMMEDIATE_S8;
        instr.condition = condition;
        return instr;
    }

    consteval DecodedInstruction push(Registers reg) {
        return DecodedInstruction{.src = ArgumentInfo{.src = ArgSrc::DOUBLE_REGISTER, .reg = reg},
                                  .type = Type::PUSH};
    }

    consteval DecodedInstruction pop(Registers reg) {
        return DecodedInstruction{.dst = ArgumentInfo{.src = ArgSrc::DOUBLE_REGISTER, .reg = reg},
                                  .type = Type::POP};
    }

    consteval DecodedInstruction add16(Registers reg, bool add_to_sp = false) {
        DecodedInstruction instr{.type = Type::ADD};
        if (add_to_sp) {
            instr.src = ArgumentInfo{.src = ArgSrc::IMMEDIATE_S8};
            instr.dst = ArgumentInfo{.src = ArgSrc::DOUBLE_REGISTER, .reg = Registers::SP};
        } else {
            instr.dst = ArgumentInfo{.src = ArgSrc::DOUBLE_REGISTER, .reg = Registers::HL};
            instr.src = ArgumentInfo{.src = ArgSrc::DOUBLE_REGISTER, .reg = reg};
        }
        return instr;
    }

    consteval DecodedInstruction ldImm8(Registers reg) {
        return DecodedInstruction{
            .ld_subtype = LoadSubtype::TYPICAL,
            .src = ArgumentInfo{.src = ArgSrc::IMMEDIATE_U8},
            .dst = ArgumentInfo{.src = reg == Registers::HL ? ArgSrc::INDIRECT : ArgSrc::REGISTER,
                                .reg = reg},
            .type = Type::LD};
    }

    consteval DecodedInstruction ldIndirectA(Registers reg, bool load_from_a, bool dec_hl = false,
                                             bool indirect_immediate = false) {
        DecodedInstruction instr{.ld_subtype = LoadSubtype::TYPICAL,
                                 .dst = ArgumentInfo{.src = ArgSrc::REGISTER, .reg = Registers::A},
                                 .type = Type::LD};
        if (indirect_immediate) {
            instr.src = ArgumentInfo{.src = ArgSrc::IMMEDIATE_U16};
        } else {
            instr.src = ArgumentInfo{.src = ArgSrc::INDIRECT, .reg = reg};
            if (reg == Registers::HL) {
                instr.ld_subtype = dec_hl ? LoadSubtype::LD_DEC : LoadSubtype::LD_INC;
            }
        }

        if (load_from_a) {
            std::swap(instr.src, instr.dst);
        }

        return instr;
    }

    consteval DecodedInstruction ldIO(bool load_from_a, bool from_immediate) {
        DecodedInstruction instr{.ld_subtype = LoadSubtype::LD_IO,
                                 .dst = ArgumentInfo{.src = ArgSrc::REGISTER, .reg = Registers::A},
                                 .type = Type::LD};

        if (from_immediate) {
            instr.src = ArgumentInfo{.src = ArgSrc::IMMEDIATE_U8};
        } else {
            instr.src = ArgumentInfo{.src = ArgSrc::REGISTER, .reg = Registers::C};
        }
        if (load_from_a) {
            std::swap(instr.src, instr.dst);
        }
        return instr;
    }

    consteval DecodedInstruction ld16(Registers reg, bool ld_sp_hl = false,
                                      bool ld_offset_sp = false, bool ld_sp_indirect = false) {
        DecodedInstruction instr{.ld_subtype = LoadSubtype::TYPICAL, .type = Type::LD};
        if (ld_sp_hl) {
            instr.dst = ArgumentInfo{.src = ArgSrc::DOUBLE_REGISTER, .reg = Registers::SP};
            instr.src = ArgumentInfo{.src = ArgSrc::DOUBLE_REGISTER, .reg = Registers::HL};
        } else if (ld_offset_sp) {
            instr.ld_subtype = LoadSubtype::LD_OFFSET_SP;
            instr.src = ArgumentInfo{.src = ArgumentSource::IMMEDIATE_S8};
            instr.dst = ArgumentInfo{.src = ArgumentSource::DOUBLE_REGISTER, .reg = Registers::HL};
        } else if (ld_sp_indirect) {
            instr.dst = ArgumentInfo{.src = ArgumentSource::IMMEDIATE_U16};
            instr.src = ArgumentInfo{.src = ArgumentSource::DOUBLE_REGISTER, .reg = Registers::SP};
            instr.ld_subtype = LoadSubtype::LD_SP;
        } else {
            instr.dst = ArgumentInfo{.src = ArgumentSource::DOUBLE_REGISTER, .reg = reg};
            instr.src = ArgumentInfo{.src = ArgumentSource::IMMEDIATE_U16};
        }

        return instr;
    }

    consteval DecodedInstruction ret(std::optional<Conditions> condition = {},
                                     bool is_reti = false) {
        DecodedInstruction instr{.type = is_reti ? Type::RETI : Type::RET};
        instr.condition = condition;
        return instr;
    }

    constexpr std::array g_top_instructions = {
        DecodedInstruction{.type = Type::NOP},
        ld16(Registers::BC),
        ldIndirectA(Registers::BC, true),
        inc(Registers::BC, true),
        inc(Registers::B),
        dec(Registers::B),
        ldImm8(Registers::B),
        DecodedInstruction{.type = Type::RLCA},
        ld16(Registers::SP, false, false, true),
        add16(Registers::BC),
        ldIndirectA(Registers::BC, false),
        dec(Registers::BC, true),
        inc(Registers::C),
        dec(Registers::C),
        ldImm8(Registers::C),
        DecodedInstruction{.type = Type::RRCA},

        DecodedInstruction{.type = Type::STOP},
        ld16(Reg::DE),
        ldIndirectA(Registers::DE, true),
        inc(Registers::DE, true),
        inc(Registers::D),
        dec(Registers::D),
        ldImm8(Registers::D),
        DecodedInstruction{.type = Type::RLA},
        jr(),
        add16(Registers::DE),
        ldIndirectA(Registers::DE, false),
        dec(Registers::DE, true),
        inc(Registers::E),
        dec(Registers::E),
        ldImm8(Registers::E),
        DecodedInstruction{.type = Type::RRA},

        jr(Conditions::NOT_ZERO),
        ld16(Registers::HL),
        ldIndirectA(Registers::HL, true),
        inc(Registers::HL, true),
        inc(Registers::H),
        dec(Registers::H),
        ldImm8(Registers::H),
        DecodedInstruction{.type = Type::DAA},
        jr(Conditions::ZERO),
        add16(Registers::HL),
        ldIndirectA(Registers::HL, false),
        dec(Registers::HL, true),
        inc(Registers::L),
        dec(Registers::L),
        ldImm8(Registers::L),
        DecodedInstruction{.type = Type::CPL},

        jr(Conditions::NOT_CARRY),
        ld16(Registers::SP),
        ldIndirectA(Registers::HL, true, true),
        inc(Registers::SP, true),
        inc(Registers::HL),
        dec(Registers::HL),
        ldImm8(Registers::HL),
        DecodedInstruction{.type = Type::SCF},
        jr(Conditions::CARRY),
        add16(Registers::SP),
        ldIndirectA(Registers::HL, false, true),
        dec(Registers::SP, true),
        inc(Registers::A),
        dec(Registers::A),
        ldImm8(Registers::A),
        DecodedInstruction{.type = Type::CCF},
    };

    constexpr std::array g_bottom_instructions = {
        ret(Conditions::NOT_ZERO),
        pop(Registers::BC),
        jp(Conditions::NOT_ZERO),
        jp(),
        call(Conditions::NOT_ZERO),
        push(Registers::BC),
        DecodedInstruction{}, // ADD n,
        rst(0),
        ret(Conditions::ZERO),
        ret(),
        jp(Conditions::ZERO),
        DecodedInstruction{}, // 0xCB
        call(Conditions::ZERO),
        call(),
        DecodedInstruction{}, // ADC n,
        rst(8),

        ret(Conditions::NOT_CARRY),
        pop(Registers::DE),
        jp(Conditions::NOT_CARRY),
        DecodedInstruction{}, // invalid
        call(Conditions::NOT_CARRY),
        push(Registers::DE),
        DecodedInstruction{}, // SUB n
        rst(0x10),
        ret(Conditions::CARRY),
        ret({}, true),
        jp(Conditions::CARRY),
        DecodedInstruction{}, // invalid
        call(Conditions::CARRY),
        DecodedInstruction{}, // invalid
        DecodedInstruction{}, // SBC n
        rst(0x18),

        ldIO(true, true),
        pop(Registers::HL),
        ldIO(true, false),
        DecodedInstruction{}, // invalid
        DecodedInstruction{}, // invalid
        push(Registers::HL),
        DecodedInstruction{}, // AND n
        rst(0x20),
        add16(Registers::SP, true),
        jp({}, true),
        ldIndirectA(Registers::NONE, true, false, true),
        DecodedInstruction{}, // invalid
        DecodedInstruction{}, // invalid
        DecodedInstruction{}, // invalid
        DecodedInstruction{}, // XOR n
        rst(0x28),

        ldIO(false, true),
        pop(Registers::AF),
        ldIO(false, false),
        DecodedInstruction{.type = Type::DI},
        DecodedInstruction{}, // invalid
        push(Registers::AF),
        DecodedInstruction{}, // OR n
        rst(0x30),
        ld16(Registers::SP, false, true),
        ld16(Registers::SP, true),
        ldIndirectA(Registers::NONE, false, false, true),
        DecodedInstruction{.type = Type::EI},
        DecodedInstruction{}, // invalid
        DecodedInstruction{}, // invalid
        DecodedInstruction{}, // CP n
        rst(0x38),

    };

    constexpr size_t i = sizeof(g_bottom_instructions);

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

    consteval uint8_t getColumnID(uint8_t quarter, uint8_t column) {
        return (quarter << 6) | column;
    }

    DecodedInstruction decodeUnprefixed(Opcode code) {
        DecodedInstruction result;

        switch (code.getX()) {
        case 0:
            result = g_top_instructions[code.code];
            break;
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
            } else {
                result = g_bottom_instructions[code.code & 0b00111111];
            }
            break;
        }

        if (result.type == Type::NONE) {
            throw std::invalid_argument("illegal instruction");
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

    constexpr void setRegisterInfo(uint8_t register_index, ArgumentInfo &register_info) {
        register_info.reg = g_byte_registers[register_index];
        register_info.src = register_info.reg == Registers::HL ? ArgumentSource::INDIRECT
                                                               : ArgumentSource::REGISTER;
    }

    constexpr void setALUInfo(Opcode code, DecodedInstruction &instruction, bool has_immediate) {
        instruction.type = g_alu[code.getY()];
        instruction.dst.src = ArgSrc::REGISTER;
        instruction.dst.reg = Reg::A;

        if (has_immediate) {
            instruction.src.src = ArgSrc::IMMEDIATE_U8;
        } else {
            setRegisterInfo(code.getZ(), instruction.src);
        }
    }

} // namespace gb::cpu
