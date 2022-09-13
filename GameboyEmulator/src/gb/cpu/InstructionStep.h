#pragma once
#include "gb/AddressBus.h"
#include "gb/cpu/CPUUtils.h"

#include <cstdint>

namespace gb {
    namespace cpu {

        class InstructionStep {
        public:
            virtual ~InstructionStep() = default;

            const bool has_mem_access_;

            virtual void execute() = 0;
        protected:
            InstructionStep(bool has_mem_access = false)
                : has_mem_access_(has_mem_access)
            {}
        };

        class ReadByte : InstructionStep {
        public:
            ReadByte(AddressBus& bus, uint16_t addr, uint8_t& dst)
                : InstructionStep(true), bus_(bus), addr_(addr), dst_(dst) {}
            
            void execute() override {
                dst_ = bus_.read(addr_);
            }

        private:
            AddressBus& bus_;
            uint16_t addr_;
            uint8_t& dst_;
        };

        class WriteByte : InstructionStep {
        public:
            WriteByte(AddressBus& bus, uint16_t addr, uint8_t data)
                : InstructionStep(true), bus_(bus), addr_(addr), data_(data) {}
            
            void execute() override {
                bus_.write(addr_, data_);
            }
        private:
            AddressBus& bus_;
            uint16_t addr_;
            uint8_t data_;
        };

        class NOP : InstructionStep {
        public:
            NOP() = default;

            void execute() override {}
        };

        class EI : InstructionStep {
        public:
            EI(bool& enableIME)
                : enableIME_(enableIME) {}
            
            void execute() override {
                enableIME_ = true;
            }
        private:
            bool& enableIME_;
        };

        class DI : InstructionStep {
        public:
            DI(bool& IME)
                : IME_(IME) {}

            void execute() override {
                IME_ = true;
            }
            
        private:
            bool& IME_;
        };

        template<typename T>
        class LD : InstructionStep {
        public:
            LD(T& dst, T data)
                : data_(data), dst_(dst) {}

            void execute() override {
                dst_ = data_;
            }
        private:
            T data_;
            T& dst_;
        };

        using LDByte = LD<uint8_t>;
        using LDWord = LD<uint16_t>;

        class INCByte : InstructionStep {
        public:
            INCByte(uint8_t& value, FlagsRegister flags)
                : value_(value), flags_(flags) {}

            void execute() override {
                flags_.set(Flags::HALF_CARRY, halfCarried(value_, 1));
                flags_.set(Flags::NEGATIVE, false);
                ++value_;
                flags_.set(Flags::ZERO, value_ == 0);
            }
        private:
            uint8_t& value_;
            FlagsRegister flags_;
        };

        class DECByte : InstructionStep {
        public:
            DECByte(uint8_t& value, FlagsRegister flags)
                : value_(value), flags_(flags) {}

            void execute() override {
                flags_.set(Flags::HALF_CARRY, halfBorrowed(value_, 1));
                flags_.set(Flags::NEGATIVE, true);
                --value_;
                flags_.set(Flags::ZERO, value_ == 0);
            }
        private:
            uint8_t& value_;
            FlagsRegister flags_;
        };

        class INCWord : InstructionStep {
        public:
            INCWord(uint16_t& value)
                : value_(value) {}

            void execute() override {
                ++value_;
            }
        private:
            uint16_t& value_;
        };

        class DECWord : InstructionStep {
        public:
            DECWord(uint16_t& value)
                : value_(value) {}

            void execute() override {
                --value_;
            }
        private:
            uint16_t& value_;
        };

        using ALUFunc = void(*)(FlagsRegister, uint8_t&, const uint8_t&);

        template<ALUFunc>
        class ALUOperation : InstructionStep {
        public:
            ALUOperation(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value)
                : flags_(flags), accumulator_(accumulator), value_(value) {}

            void execute() override {
                ALUFunc(flags_, accumulator_, value_);
            }
        protected:
            FlagsRegister flags_;
            uint8_t& accumulator_;
            const uint8_t& value_;
        };

        void ALUAdd(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value);
        void ALUSub(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value);
        void ALUXor(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value);
        void ALUOr(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value);
        void ALUAnd(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value);
        void ALUSBC(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value);
        void ALUAdc(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value);
        void ALUCp(FlagsRegister flags, uint8_t& accumulator, const uint8_t& value);

        using ADDByte = ALUOperation<ALUAdd>;
        using SUB = ALUOperation<ALUSub>;
        using XOR = ALUOperation<ALUXor>;
        using OR = ALUOperation<ALUOr>;
        using AND = ALUOperation<ALUAnd>;
        using SBC = ALUOperation<ALUSbc>;
        using ADC = ALUOperation<ALUAdc>;
        using CP = ALUOperation<ALUCp>;

        class ADDWord : InstructionStep {
        public:
            ADDWord(FlagsRegister flags, uint16_t& accumulator, uint16_t value)
                : flags_(flags), accumulator_(accumulator), value_(value) {}

            void execute() override;

        private:
            FlagsRegister flags_;
            uint16_t& accumulator_;
            uint16_t value_;
        };

        class ADDSP : InstructionStep {
        public:
            ADDSP(FlagsRegister flags, uint16_t stack_pointer, int8_t value)
                : flags_(flags), stack_pointer_(stack_pointer), value_(value) {}

            void execute() override;

        private:
            FlagsRegister flags_;
            uint16_t& stack_pointer_;
            int8_t& value_;
        };

        class Jump : InstructionStep {
        public:
            Jump(uint16_t& program_counter, const uint16_t& address)
                : program_counter_(program_counter), address_(address) {}

            void execute() override {
                program_counter_ = address_;
            }
            
        private:
            uint16_t& program_counter_;
            const uint16_t& address_;
        };
    }
}
