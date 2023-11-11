#pragma once
#include "gb/address_bus.h"
#include "gb/cpu/cpu_utils.h"
#include "gb/cpu/decoder.h"
#include "gb/cpu/operation.h"
#include "gb/interrupt_register.h"
#include "util/util.h"

#include <cstdint>
#include <optional>

namespace gb::cpu {

    constexpr inline uint16_t getInterruptVector(InterruptFlags interrupt) {
        switch (interrupt) {
        case InterruptFlags::VBLANK: return 0x40;
        case InterruptFlags::LCD_STAT: return 0x48;
        case InterruptFlags::TIMER: return 0x50;
        case InterruptFlags::SERIAL: return 0x58;
        case InterruptFlags::JOYPAD: return 0x60;
        }
    }

    struct Instruction {
        using Argument = Variant<std::monostate, Registers, int8_t, uint8_t, uint16_t>;

        InstructionType type = InstructionType::NONE;
        std::optional<LoadSubtype> load_subtype;
        std::optional<Conditions> condition;

        Argument src;
        Argument dst;

        RegisterFile registers;
        bool ime = false;

        Argument &arg() { return src; }
        Argument &arg1() { return src; }
        Argument &arg2() { return dst; }
        Argument &bit() { return dst; }
    };

    struct MemoryOp {
        enum class Type : uint8_t { NONE, READ, FETCH_INSTRUCTION, WRITE };

        uint16_t address = 0;
        Type type = Type::NONE;
        uint8_t data;
    };

    class DataBuffer {
      public:
        DataBuffer() = default;

        void put(uint8_t data) {
            lsb_ = msb_;
            msb_ = data;
        }
        void putHigh(uint8_t data) { msb_ = data; }
        void putLow(uint8_t data) { lsb_ = data; }

        uint8_t get() { return msb_; }
        int8_t getSigned() { return int8_t(msb_); }
        uint16_t getWord() { return uint16_t(lsb_) | (uint16_t(msb_) << 8); }

        uint8_t *highPtr() { return &msb_; }
        uint8_t *lowPtr() { return &lsb_; }
        uint8_t *ptr() { return &lsb_; }

      private:
        uint8_t lsb_ = 0;
        uint8_t msb_ = 0;
        bool empty_ = true;
    };
    class SharpSM83 {
      public:
        SharpSM83(AddressBus &bus, InterruptRegister &interrupt_enable, InterruptRegister &interrupt_flags);

        void tick();

        RegisterFile getRegisters() const { return reg_; }

        uint16_t getProgramCounter() const { return reg_.PC(); }

        bool isFinished() const { return stopped_ || finished_; }

        bool isHalted() const { return halt_mode_; }

        bool isStopped() const { return stopped_; }

        Instruction getLastInstruction() const { return last_instruction_; }

        void reset();

      private:
        void dispatch();

        std::optional<InterruptFlags> getPendingInterrupt() const;
        void handleInterrupt(InterruptFlags interrupt);

        uint8_t getByteRegister(Registers reg);

        uint16_t getWordRegister(Registers reg) const;

        void setByteRegister(Registers reg, uint8_t data);

        void setWordRegister(Registers reg, uint16_t data);

        bool checkCondition(Conditions condition);

        void setArgData(Instruction::Argument &arg, ArgumentInfo info, uint8_t data);

        void sheduleMemoryNoOp();
        void sheduleReadByte(uint16_t address);
        void sheduleReadWord(uint16_t address);
        void sheduleWriteByte(uint16_t address, uint8_t data);
        void sheduleWriteWord(uint16_t address, uint16_t data);
        void shedulePushStack(uint16_t data);
        void shedulePopStack(Registers reg);
        void sheduleMemoryAcceses(DecodedInstruction instr);
        void sheduleReadToReg(uint16_t address, Registers reg);
        void sheduleFetchInstruction();
        void executeMemoryOp();

        void decode(Opcode code);
        void pushMemoryOp(MemoryOp op);

        // Unprefixed instrictions. Return the amount of machine cycles needed
        // for the instruction
        void NOP() {}
        void RLA();
        void RLCA();
        void RRA();
        void RRCA();
        void DI();
        void CPL();
        void RETI();
        void CCF();
        void EI();
        void DAA();
        void HALT();
        void SCF();
        void STOP();
        void PUSH(Registers reg);
        void POP(Registers reg);
        void RST(uint16_t reset_vector);
        void CALL(std::optional<Conditions> condition);
        void JR(std::optional<Conditions> condition);
        void RET(std::optional<Conditions> condition);
        void INC(ArgumentInfo target);
        void DEC(ArgumentInfo target);
        void SUB(ArgumentInfo argument);
        void OR(ArgumentInfo argument);
        void AND(ArgumentInfo argument);
        void XOR(ArgumentInfo argument);
        void ADC(ArgumentInfo argument);
        void SBC(ArgumentInfo argument);
        void CP(ArgumentInfo argument);
        void JP(DecodedInstruction instr);
        void LD(DecodedInstruction instr);
        void ADD(DecodedInstruction instr);

        void NONE() {}

        void loadByte(ArgumentInfo destination, ArgumentInfo source);

        // Prefixed instructions. Return the amount of machine cycles needed for
        // the instruction
        void RLC(Registers reg);
        void RRC(Registers reg);
        void RL(Registers reg);
        void RR(Registers reg);
        void SLA(Registers reg);
        void SRA(Registers reg);
        void SWAP(Registers reg);
        void SRL(Registers reg);

        void BIT(Registers reg, uint8_t bit);
        void RES(Registers reg, uint8_t bit);
        void SET(Registers reg, uint8_t bit);

      private:
        AddressBus &bus_;
        InterruptRegister &ie_;
        InterruptRegister &if_;

        RegisterFile reg_;

        bool IME_ = false; // Interrupt master enable
        bool enable_IME_ = false;
        bool halt_mode_ = false;
        bool halt_bug_ = false;
        bool memory_op_executed_ = false;
        bool prefixed_next_ = false;
        bool stopped_ = false;
        bool finished_ = false;

        Queue<MemoryOp, 8> memory_op_queue_;
        Instruction last_instruction_;
        Instruction instruction_;
        DataBuffer data_buffer_;
        std::optional<DecodedInstruction> current_instruction_;
    };
} // namespace gb::cpu
