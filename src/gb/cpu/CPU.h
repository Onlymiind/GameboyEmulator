#pragma once
#include "utils/Utils.h"
#include "gb/AddressBus.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Operation.h"
#include "gb/cpu/Decoder.h"
#include "gb/cpu/CPUUtils.h"

#include <cstdint>
#include <limits>
#include <unordered_map>
#include <optional>
#include <variant>

namespace gb::cpu {

    inline const std::unordered_map<InterruptFlags, uint16_t> g_interrupt_vectors = {
        {InterruptFlags::VBlank,   0x0040},
        {InterruptFlags::LCD_STAT, 0x0048},
        {InterruptFlags::Timer,    0x0050},
        {InterruptFlags::Serial,   0x0058},
        {InterruptFlags::Joypad,   0x0060}
    };

    struct Instruction {
        using Argument = Variant<std::monostate, decoding::Registers, int8_t, uint8_t, uint16_t>;

        decoding::InstructionType type = decoding::InstructionType::None;
        std::optional<decoding::LoadSubtype> load_subtype;
        std::optional<decoding::Conditions> condition;

        Argument src;
        Argument dst;

        Argument& arg() { return src; }
        Argument& arg1() { return src; }
        Argument& arg2() { return dst; }
        Argument& bit() { return dst; }
    };
    
    class SharpSM83 {
    public:
        SharpSM83(const AddressBus& bus, InterruptRegister& interruptEnable, InterruptRegister& interruptFlags);
        ~SharpSM83() {}

        void tick();

        RegisterFile getRegisters() const { return reg_; }

        inline uint16_t getProgramCounter() const { return reg_.PC; }

        inline bool isFinished() const { return cycles_to_finish_ == 0; }

        Instruction getLastInstruction() const { return last_instruction_; }

        void reset();

    private:

        uint8_t dispatch(decoding::opcode code);
        uint8_t dispatchPrefixed(decoding::PrefixedInstruction instr);
        uint8_t dispatchUnprefixed(decoding::UnprefixedInstruction instr);

        std::optional<InterruptFlags> getPendingInterrupt() const;
        void handleInterrupt(InterruptFlags interrupt);

        uint8_t fetch();

        uint16_t fetchWord();

        int8_t fetchSigned();

        void pushStack(uint16_t value);

        uint16_t popStack();

        uint8_t getByte(decoding::ArgumentInfo from);
        uint16_t getWord(decoding::ArgumentInfo from);

        uint8_t getByteRegister(decoding::Registers reg) const;

        uint16_t getWordRegister(decoding::Registers reg) const;

        void setByteRegister(decoding::Registers reg, uint8_t data);

        void setWordRegister(decoding::Registers reg, uint16_t data);

        bool checkCondition(decoding::Conditions condition);

        void set_arg_data(Instruction::Argument& arg, decoding::ArgumentInfo info, uint8_t data);


        //Unprefixed instrictions. Return the amount of machine cycles needed for the instruction
        uint8_t NOP() { return 1; }; 
        uint8_t RLA(); 
        uint8_t RLCA();
        uint8_t RRA(); 
        uint8_t RRCA();
        uint8_t DI(); 
        uint8_t CPL();
        uint8_t RETI();
        uint8_t CCF();
        uint8_t EI();
        uint8_t DAA(); 
        uint8_t HALT(); 
        uint8_t SCF(); 
        uint8_t STOP();
        uint8_t PUSH(decoding::Registers reg);
        uint8_t POP(decoding::Registers reg); 
        uint8_t RST(uint16_t reset_vector);
        uint8_t CALL(std::optional<decoding::Conditions> condition);
        uint8_t JR(std::optional<decoding::Conditions> condition); 
        uint8_t RET(std::optional<decoding::Conditions> condition); 
        uint8_t INC(decoding::ArgumentInfo target);
        uint8_t DEC(decoding::ArgumentInfo target);
        uint8_t SUB(decoding::ArgumentInfo argument); 
        uint8_t OR(decoding::ArgumentInfo argument); 
        uint8_t AND(decoding::ArgumentInfo argument); 
        uint8_t XOR(decoding::ArgumentInfo argument); 
        uint8_t ADC(decoding::ArgumentInfo argument);
        uint8_t SBC(decoding::ArgumentInfo argument); 
        uint8_t CP(decoding::ArgumentInfo argument); 
        uint8_t JP(decoding::UnprefixedInstruction instr); 
        uint8_t LD(decoding::UnprefixedInstruction instr);
        uint8_t ADD(decoding::UnprefixedInstruction instr);

        uint8_t NONE() { return 0; };

        uint8_t loadByte(decoding::ArgumentInfo destination, decoding::ArgumentInfo source);

        //Prefixed instructions. Return the amount of machine cycles needed for the instruction
        uint8_t RLC (decoding::Registers reg); uint8_t RRC(decoding::Registers reg); 
        uint8_t RL  (decoding::Registers reg); uint8_t RR (decoding::Registers reg);
        uint8_t SLA (decoding::Registers reg); uint8_t SRA(decoding::Registers reg);
        uint8_t SWAP(decoding::Registers reg); uint8_t SRL(decoding::Registers reg);

        uint8_t BIT (decoding::PrefixedInstruction instr); uint8_t RES(decoding::PrefixedInstruction instr);  
        uint8_t SET (decoding::PrefixedInstruction instr);


    private:
        InterruptRegister& interrupt_enable_;
        InterruptRegister& interrupt_flags_;
        const AddressBus& bus_;

        RegisterFile reg_;
        uint8_t cycles_to_finish_;

        bool IME_ = false; // Interrupt master enable
        bool enable_IME_ = false;
        bool halt_mode_ = false;
        bool halt_bug_ = false;

        Instruction last_instruction_;
    };
}
