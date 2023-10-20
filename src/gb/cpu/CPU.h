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
        using Argument = Variant<std::monostate, Registers, int8_t, uint8_t, uint16_t>;

        InstructionType type = InstructionType::None;
        std::optional<LoadSubtype> load_subtype;
        std::optional<Conditions> condition;

        Argument src;
        Argument dst;
        uint16_t pc = 0;

        Argument& arg() { return src; }
        Argument& arg1() { return src; }
        Argument& arg2() { return dst; }
        Argument& bit() { return dst; }
    };

    struct MemoryOp {
        enum class Type : uint8_t {
            NONE, READ, READ_LOW, READ_HIGH, WRITE
        };

        uint16_t address = 0;
        Type type = Type::NONE;
        Variant<std::monostate, uint8_t, Registers> data;
    };
    
    class DataBuffer {
    public:
        DataBuffer() = default;

        bool empty() const { return empty_; }
        void put(uint8_t data) { lsb_ = data; empty_ = false; }
        void putHigh(uint8_t data) { msb_ = data; empty_ = false; }
        void putLow(uint8_t data) { lsb_ = data; empty_ = false; }

        uint8_t get() { empty_ = true; return lsb_; }
        int8_t getSigned() { empty_ = true; return std::bit_cast<int8_t>(lsb_); }
        uint16_t getWord() { empty_ = true; return uint16_t(lsb_) | (uint16_t(msb_) << 8); }

        uint8_t* highPtr() { return &msb_; }
        uint8_t* lowPtr() { return &lsb_; }
        uint8_t* ptr() { return &lsb_; }

    private:
        uint8_t lsb_ = 0;
        uint8_t msb_ = 0;
        bool empty_ = true;
    };
    class SharpSM83 {
    public:
        SharpSM83(AddressBus& bus);
        ~SharpSM83() {}

        void tick();

        RegisterFile getRegisters() const { return reg_; }

        inline uint16_t getProgramCounter() const { return reg_.PC; }

        inline bool isFinished() const { 
            return !current_instruction_ && !prefixed_next_ && memory_op_queue_.empty();
            //(memory_op_queue_.size() == 1 || ( && memory_op_executed_));
        }

        inline bool isHalted() const { return halt_mode_; }

        Instruction getLastInstruction() const { return last_instruction_; }

        void reset();

    private:

        uint8_t dispatch();

        std::optional<InterruptFlags> getPendingInterrupt() const;
        void handleInterrupt(InterruptFlags interrupt);

        uint8_t getByteRegister(Registers reg);

        uint16_t getWordRegister(Registers reg) const;

        void setByteRegister(Registers reg, uint8_t data);

        void setWordRegister(Registers reg, uint16_t data);

        bool checkCondition(Conditions condition);

        void setArgData(Instruction::Argument& arg, ArgumentInfo info, uint8_t data);

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

        void decode(opcode code);
        void pushMemoryOp(MemoryOp op);

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
        uint8_t PUSH(Registers reg);
        uint8_t POP(Registers reg); 
        uint8_t RST(uint16_t reset_vector);
        uint8_t CALL(std::optional<Conditions> condition);
        uint8_t JR(std::optional<Conditions> condition); 
        uint8_t RET(std::optional<Conditions> condition); 
        uint8_t INC(ArgumentInfo target);
        uint8_t DEC(ArgumentInfo target);
        uint8_t SUB(ArgumentInfo argument); 
        uint8_t OR(ArgumentInfo argument); 
        uint8_t AND(ArgumentInfo argument); 
        uint8_t XOR(ArgumentInfo argument); 
        uint8_t ADC(ArgumentInfo argument);
        uint8_t SBC(ArgumentInfo argument); 
        uint8_t CP(ArgumentInfo argument); 
        uint8_t JP(DecodedInstruction instr); 
        uint8_t LD(DecodedInstruction instr);
        uint8_t ADD(DecodedInstruction instr);

        uint8_t NONE() { return 0; };

        uint8_t loadByte(ArgumentInfo destination, ArgumentInfo source);

        //Prefixed instructions. Return the amount of machine cycles needed for the instruction
        uint8_t RLC (Registers reg); uint8_t RRC(Registers reg); 
        uint8_t RL  (Registers reg); uint8_t RR (Registers reg);
        uint8_t SLA (Registers reg); uint8_t SRA(Registers reg);
        uint8_t SWAP(Registers reg); uint8_t SRL(Registers reg);

        uint8_t BIT (Registers reg, uint8_t bit); uint8_t RES(Registers reg, uint8_t bit);  
        uint8_t SET (Registers reg, uint8_t bit);


    private:
        AddressBus& bus_;

        RegisterFile reg_;
        uint8_t cycles_to_finish_ = 0;
        uint16_t last_pc_ = 0;

        bool IME_ = false; // Interrupt master enable
        bool enable_IME_ = false;
        bool halt_mode_ = false;
        bool halt_bug_ = false;
        bool memory_op_executed_ = false;
        bool prefixed_next_ = false;
        bool wait_for_pc_read_ = false;

        Queue<MemoryOp, 8> memory_op_queue_;
        Instruction last_instruction_;
        DataBuffer data_buffer_;
        std::optional<DecodedInstruction> current_instruction_;
    };
}
