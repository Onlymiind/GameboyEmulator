#ifndef GB_EMULATOR_SRC_GB_CPU_OPERATION_HDR_
#define GB_EMULATOR_SRC_GB_CPU_OPERATION_HDR_

#include <cstdint>
#include <iostream>
#include <optional>

namespace gb::cpu {

    enum class InstructionType : uint8_t {
        NONE = 0,
        NOP,
        LD,
        INC,
        RLA,
        RLCA,
        ADD,
        JR,
        DEC,
        RRA,
        RRCA,
        SUB,
        OR,
        AND,
        XOR,
        PUSH,
        ADC,
        JP,
        POP,
        RST,
        CALL,
        SBC,
        DI,
        RET,
        CPL,
        RETI,
        CCF,
        EI,
        DAA,
        SCF,
        HALT,
        CP,
        STOP,

        RLC,
        RRC,
        SLA,
        SRA,
        SRL,
        BIT,
        RES,
        SET,
        RL,
        RR,
        SWAP
    };

    // Some LD instructions are quite different fron others, this enum is used
    // to mark them
    enum class LoadSubtype : uint8_t { TYPICAL = 0, LD_INC, LD_DEC, LD_IO, LD_SP, LD_OFFSET_SP };

    enum class ArgumentSource : uint8_t {
        NONE = 0,
        REGISTER,
        DOUBLE_REGISTER,
        INDIRECT,
        IMMEDIATE_U8,
        IMMEDIATE_U16,
        IMMEDIATE_S8
    };

    enum class ArgumentType : uint8_t { NONE = 0, UNSIGNED_8, UNSIGNED_16, SIGNED_8 };

    enum class Registers : uint8_t {

        FLAGS = 0,
        A = 1,
        C = 2,
        B = 3,
        E = 4,
        D = 5,
        L = 6,
        H = 7,
        PC_LOW = 8,
        PC_HIGH = 9,
        AF = A << 4,
        BC = (B << 4) | C,
        DE = (D << 4) | E,
        HL = (H << 4) | L,
        PC = (PC_HIGH << 4) | PC_LOW,
        SP = 0xfd,
        NONE = 0xff,

        LOW_REG_MASK = 0x0F,
        HIGH_REG_MASK = 0xF0
    };

    constexpr inline bool isByteRegister(Registers reg) { return (uint8_t(reg) & 0xf0) == 0; }

    constexpr inline uint8_t operator&(Registers lhs, Registers rhs) { return uint8_t(lhs) & uint8_t(rhs); }

    enum class Conditions : uint8_t { NOT_ZERO, ZERO, NOT_CARRY, CARRY };

    struct ArgumentInfo {
        ArgumentSource src = ArgumentSource::NONE;
        Registers reg = Registers::NONE;

        inline bool operator==(ArgumentInfo other) const { return src == other.src && reg == other.reg; }
    };

    struct DecodedInstruction {
        // Used only in RST instruction
        std::optional<uint16_t> reset_vector;
        // Used only in LD instructions
        std::optional<LoadSubtype> ld_subtype;

        std::optional<Conditions> condition;
        ArgumentInfo src;
        ArgumentInfo dst;
        InstructionType type = InstructionType::NONE;
        // Used only in prefixed instructions
        std::optional<uint8_t> bit;

        // For Debugging
        inline bool operator==(DecodedInstruction other) const {
            return type == other.type && src == other.src && dst == other.dst && condition == other.condition &&
                   reset_vector == other.reset_vector && ld_subtype == other.ld_subtype && bit == other.bit;
        }

        constexpr ArgumentInfo &arg() { return src; }
    };

    // Struct for easy opcode decomposition;
    struct Opcode {
        constexpr Opcode() : code(0) {}
        constexpr Opcode(uint8_t val) : code(val) {}

        constexpr inline uint8_t getX() const { return (code & 0b11000000) >> 6; }
        constexpr inline uint8_t getY() const { return (code & 0b00111000) >> 3; }
        constexpr inline uint8_t getZ() const { return (code & 0b00000111) >> 0; }
        constexpr inline uint8_t getP() const { return (code & 0b00110000) >> 4; }
        constexpr inline uint8_t getQ() const { return (code & 0b00001000) >> 3; }
        constexpr inline uint8_t getColumn() const { return code & 0b1100'1111; }

        constexpr inline uint8_t getLowerNibble() const { return code & 0x0F; }

        uint8_t code;
    };

    std::string_view to_string(InstructionType type);
    std::string_view to_string(Registers reg);
    std::string_view to_string(Conditions cond);
    // TODO
    //  std::ostream& operator<<(std::ostream& os, LoadSubtype subtype);
    //  std::ostream& operator<<(std::ostream& os, ArgumentSource source);
    //  std::ostream& operator<<(std::ostream& os, ArgumentType arg_type);
    //  std::ostream& operator<<(std::ostream& os, Conditions condition);
    //  std::ostream& operator<<(std::ostream& os, ArgumentInfo arg_info);
    //  std::ostream& operator<<(std::ostream& os, UnprefixedInstruction instr);
    //  std::ostream& operator<<(std::ostream& os, PrefixedInstruction instr);
} // namespace gb::cpu

#endif
