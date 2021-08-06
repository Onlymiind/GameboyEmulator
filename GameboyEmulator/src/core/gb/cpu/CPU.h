#pragma once
#include "utils/Utils.h"
#include "core/gb/AddressBus.h"
#include "core/gb/InterruptRegister.h"
#include "core/gb/cpu/Operation.h"
#include "core/gb/cpu/Decoder.h"

#include <cstdint>
#include <string_view>
#include <string>
#include <array>
#include <map>
#include <functional>
#include <numeric>

namespace gb
{
    class SharpSM83 
    {
        using pfn_instruction = uint8_t(*)(SharpSM83&, const opcode);
    public:
        SharpSM83(const AddressBus& bus, InterruptRegister& interruptEnable, InterruptRegister& interruptFlags, const Decoder& decoder);
        ~SharpSM83() {}

        void tick();

        std::string registersOut();

        inline uint16_t getProgramCounter() const { return REG.PC; }

        inline bool isFinished() const { return m_CyclesToFinish == 0; }

        void reset();

    private:

        uint8_t dispatch(opcode code);
        uint8_t dispatchPrefixed(PrefixedInstruction instr);
        uint8_t dispatchUnprefixed(UnprefixedInstruction instr);

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

        uint8_t getByte(ArgumentInfo from);
        uint16_t getWord(ArgumentInfo from);

        uint8_t getByteRegister(Registers reg) const;

        uint16_t getWordRegister(Registers reg) const;

        void setByteRegister(Registers reg, uint8_t data);

        void setWordRegister(Registers reg, uint16_t data);

        bool checkCondition(Conditions condition);


        //Unprefixed instrictions. Return the amount of machine cycles needed for the instruction
        static uint8_t LD(SharpSM83& cpu, const opcode code); 
        static uint8_t LD_IMM(SharpSM83& cpu, const opcode code);
        static uint8_t ADD(SharpSM83& cpu, const opcode code); 
        static uint8_t LD_IO(SharpSM83& cpu, const opcode code);
        static uint8_t LD_REG8(SharpSM83& cpu, const opcode code); 
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
        uint8_t JP(UnprefixedInstruction instr); 
        uint8_t SBC(ArgumentInfo argument); 
        uint8_t CP(ArgumentInfo argument); 

        uint8_t NONE();

        //Prefixed instructions. Return the amount of machine cycles needed for the instruction
        uint8_t RLC (Registers reg); uint8_t RRC(Registers reg); 
        uint8_t RL  (Registers reg); uint8_t RR (Registers reg);
        uint8_t SLA (Registers reg); uint8_t SRA(Registers reg);
        uint8_t SWAP(Registers reg); uint8_t SRL(Registers reg);

        uint8_t BIT (PrefixedInstruction instr); uint8_t RES(PrefixedInstruction instr);  
        uint8_t SET (PrefixedInstruction instr);


    private: //REGISTERS
        struct {
            union {
                uint16_t AF = 0;
            
                struct {
                    union {
                        uint8_t Value;

                        struct {
                            uint8_t Unused : 4;
                            uint8_t C : 1;
                            uint8_t H : 1;
                            uint8_t N : 1;
                            uint8_t Z : 1;
                        };
                    }Flags;

                    uint8_t A;
                };
            };

            union {
                uint16_t BC = 0;

                struct {
                    uint8_t C;
                    uint8_t B;
                };

            };

            union {
                uint16_t DE = 0;

                struct {
                    uint8_t E;
                    uint8_t D;
                };
            };

            union {
                uint16_t HL = 0;

                struct {
                    uint8_t L;
                    uint8_t H;
                };
            };

            uint16_t SP{ 0xFFFE }, PC{ 0x0100 }; // Stack pointer, program counter
        } REG;

        bool IME{ false }; // Interrupt master enable
        bool m_EnableIME{ false };

        InterruptRegister& m_InterruptEnable;
        InterruptRegister& m_InterruptFlags;

    private: //STUFF

        const AddressBus& m_Bus;
        const Decoder& m_Decoder;

        uint8_t m_CyclesToFinish;
    };
}
