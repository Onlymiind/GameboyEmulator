#pragma once
#include "utils/Utils.h"
#include "gb/AddressBus.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Operation.h"
#include "gb/cpu/Decoder.h"
#include "gb/cpu/Registers.h"

#include <cstdint>
#include <string_view>
#include <string>
#include <array>
#include <map>
#include <functional>
#include <numeric>

namespace gb
{
    namespace cpu
    {

        class SharpSM83 
        {
        public:
            SharpSM83(const AddressBus& bus, InterruptRegister& interruptEnable, InterruptRegister& interruptFlags, const decoding::Decoder& decoder);
            ~SharpSM83() {}

            void tick();

            Registers getRegisters() const;

            inline uint16_t getProgramCounter() const { return REG.PC; }

            inline bool isFinished() const { return m_CyclesToFinish == 0; }

            void reset();

        private:

            uint8_t dispatch(decoding::opcode code);
            uint8_t dispatchPrefixed(decoding::PrefixedInstruction instr);
            uint8_t dispatchUnprefixed(decoding::UnprefixedInstruction instr);

            uint8_t read(uint16_t address) const;

            void write(uint16_t address, uint8_t data);

            uint8_t fetch();

            uint16_t fetchWord();

            int8_t fetchSigned();

            void pushStack(uint16_t value);

            uint16_t popStack();

            bool halfCarryOccured8Add(uint8_t lhs, uint8_t rhs);

            bool halfCarryOccured8Sub(uint8_t lhs, uint8_t rhs);
            
            bool halfCarryOccured16Add(uint16_t lhs, uint16_t rhs);

            template<typename T>
            bool carryOccured(T lhs, T rhs, bool substract = false) const
            {
                uint32_t a(lhs), b(rhs);
                if(substract)
                {
                    return a < b;
                }
                else
                {
                    return (a + b) > std::numeric_limits<T>::max();
                }
            }

            uint8_t getByte(decoding::ArgumentInfo from);
            uint16_t getWord(decoding::ArgumentInfo from);

            uint8_t getByteRegister(decoding::Registers reg) const;

            uint16_t getWordRegister(decoding::Registers reg) const;

            void setByteRegister(decoding::Registers reg, uint8_t data);

            void setWordRegister(decoding::Registers reg, uint16_t data);

            bool checkCondition(decoding::Conditions condition);


            //Unprefixed instrictions. Return the amount of machine cycles needed for the instruction
            uint8_t NOP(); 
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

            uint8_t NONE();

            uint8_t loadByte(decoding::ArgumentInfo destination, decoding::ArgumentInfo source);

            //Prefixed instructions. Return the amount of machine cycles needed for the instruction
            uint8_t RLC (decoding::Registers reg); uint8_t RRC(decoding::Registers reg); 
            uint8_t RL  (decoding::Registers reg); uint8_t RR (decoding::Registers reg);
            uint8_t SLA (decoding::Registers reg); uint8_t SRA(decoding::Registers reg);
            uint8_t SWAP(decoding::Registers reg); uint8_t SRL(decoding::Registers reg);

            uint8_t BIT (decoding::PrefixedInstruction instr); uint8_t RES(decoding::PrefixedInstruction instr);  
            uint8_t SET (decoding::PrefixedInstruction instr);


        private: //REGISTERS
            Registers REG;

            bool IME = false; // Interrupt master enable
            bool m_EnableIME = false;

            InterruptRegister& m_InterruptEnable;
            InterruptRegister& m_InterruptFlags;

        private: //STUFF

            const AddressBus& m_Bus;
            const decoding::Decoder& m_Decoder;

            uint8_t m_CyclesToFinish;
        };
    }
}