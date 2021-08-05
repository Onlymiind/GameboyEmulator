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
        static uint8_t NOP(); static uint8_t LD(SharpSM83& cpu, const opcode code); static uint8_t INC(SharpSM83& cpu, const opcode code); uint8_t RLA(); uint8_t RLCA();
        static uint8_t ADD(SharpSM83& cpu, const opcode code); static uint8_t JR(SharpSM83& cpu, const opcode code); static uint8_t DEC(SharpSM83& cpu, const opcode code); uint8_t RRA(); uint8_t RRCA();
        uint8_t SUB(ArgumentInfo argument); uint8_t OR(ArgumentInfo argument); uint8_t AND(ArgumentInfo argument); uint8_t XOR(ArgumentInfo argument); uint8_t PUSH(Registers reg);
        uint8_t ADC(ArgumentInfo argument); static uint8_t JP(SharpSM83& cpu, const opcode code); uint8_t POP(Registers reg); uint8_t RST(uint16_t reset_vector); static uint8_t CALL(SharpSM83& cpu, const opcode code);
        uint8_t SBC(ArgumentInfo argument); uint8_t DI(); static uint8_t RET(SharpSM83& cpu, const opcode code); uint8_t CPL(); uint8_t RETI();
        uint8_t CCF(); uint8_t EI(); uint8_t DAA(); uint8_t HALT(); static uint8_t LD_IO(SharpSM83& cpu, const opcode code);
        uint8_t SCF(); uint8_t CP(ArgumentInfo argument); uint8_t STOP(); static uint8_t LD_REG8(SharpSM83& cpu, const opcode code); static uint8_t LD_IMM(SharpSM83& cpu, const opcode code);

        static uint8_t NONE(SharpSM83& cpu, const opcode code);

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
                uint16_t AF{};
            
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
                uint16_t BC{};

                struct {
                    uint8_t C;
                    uint8_t B;
                };

            };

            union {
                uint16_t DE{};

                struct {
                    uint8_t E;
                    uint8_t D;
                };
            };

            union {
                uint16_t HL{};

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

    private: //OPCODE DECODING

        // //8-bit registers lookup
        // const std::array<uint8_t*, 8> m_TableREG8 = 
        // {
        //     reinterpret_cast<uint8_t*>(&REG.BC) + 1, reinterpret_cast<uint8_t*>(&REG.BC) + 0, // &B, &C
        //     reinterpret_cast<uint8_t*>(&REG.DE) + 1, reinterpret_cast<uint8_t*>(&REG.DE) + 0, // &D, &E
        //     reinterpret_cast<uint8_t*>(&REG.HL) + 1, reinterpret_cast<uint8_t*>(&REG.HL) + 0, // &H, &L
        //     reinterpret_cast<uint8_t*>(&REG.HL) + 0, reinterpret_cast<uint8_t*>(&REG.AF) + 1  // &HL ([HL]), &A
        // };

        // //Register pair lookup with SP
        // const std::array<uint16_t*, 4> m_TableREGP_SP = { &REG.BC, &REG.DE, &REG.HL, &REG.SP };

        // //Register pair lookup with AF
        // const std::array<uint16_t*, 4> m_TableREGP_AF = { &REG.BC, &REG.DE, &REG.HL, &REG.AF };

        //Conditions lookup
        // const std::array<Conditions, 4> m_Conditions = 
        // { 
        //     Conditions::NotZero, //Not Z-flag
        //     Conditions::Zero, //Z-flag
        //     Conditions::NotCarry, //Not C-flag
        //     Conditions::Carry  //C-flag
        // };

        // const std::array<pfn_instruction, 8> m_TableALU =
        // {
        //     SharpSM83::ADD, SharpSM83::ADC, SharpSM83::SUB, SharpSM83::SBC,
        //     SharpSM83::AND, SharpSM83::XOR, SharpSM83::OR,  SharpSM83::CP
        // };

        // const std::map<uint8_t, pfn_instruction> m_ColumnToImplUpper = 
        // {
        //     { 0x01, SharpSM83::LD_IMM }, { 0x02, SharpSM83::LD }, { 0x03, SharpSM83::INC }, { 0x04, SharpSM83::INC }, { 0x05, SharpSM83::DEC }, { 0x06, SharpSM83::LD_IMM },
        //     { 0x09, SharpSM83::ADD }, { 0x0A, SharpSM83::LD }, { 0x0B, SharpSM83::DEC }, { 0x0C, SharpSM83::INC }, { 0x0D, SharpSM83::DEC }, { 0x0E, SharpSM83::LD_IMM }
        // };

        // const std::map<uint8_t, pfn_instruction> m_ColumnToImplLower =
        // {
        //     { 0x01, SharpSM83::POP }, { 0x05, SharpSM83::PUSH }, { 0x07, SharpSM83::RST }, { 0x0F, SharpSM83::RST }
        // };

        // const std::map<uint8_t, pfn_instruction> m_RandomInstructions = 
        // {
        //     { 0x00, SharpSM83::NOP }, { 0x07, SharpSM83::RLCA }, { 0x08, SharpSM83::LD_IMM }, { 0x0F, SharpSM83::RRCA },
            
        //     { 0x10, SharpSM83::STOP }, { 0x17, SharpSM83::RLA }, { 0x18, SharpSM83::JR }, { 0x1F, SharpSM83::RRA },
            
        //     { 0x20, SharpSM83::JR }, { 0x27, SharpSM83::DAA }, { 0x28, SharpSM83::JR }, { 0x2F, SharpSM83::CPL },
            
        //     { 0x30, SharpSM83::JR }, { 0x37, SharpSM83::SCF }, { 0x38, SharpSM83::JR }, { 0x3F, SharpSM83::CCF },

        //     { 0xC0, SharpSM83::RET }, { 0xC2, SharpSM83::JP }, { 0xC3, SharpSM83::JP }, { 0xC4, SharpSM83::CALL },
        //     { 0xC8, SharpSM83::RET }, { 0xC9, SharpSM83::RET }, { 0xCA, SharpSM83::JP }, { 0xCC, SharpSM83::CALL },
        //     { 0xCD, SharpSM83::CALL }, 
            
        //     { 0xD0, SharpSM83::RET }, { 0xD2, SharpSM83::JP }, { 0xD4, SharpSM83::CALL }, { 0xD8, SharpSM83::RET },
        //     { 0xD9, SharpSM83::RETI }, {0xDA, SharpSM83::JP }, { 0xDC, SharpSM83::CALL }, 
            
        //     { 0xE0, SharpSM83::LD_IO }, { 0xE2, SharpSM83::LD_IO }, { 0xE8, SharpSM83::ADD }, { 0xE9, SharpSM83::JP }, 
        //     { 0xEA, SharpSM83::LD },
            
        //     { 0xF0, SharpSM83::LD_IO }, { 0xF2, SharpSM83::LD_IO }, { 0xF3, SharpSM83::DI }, { 0xF8, SharpSM83::LD_IMM }, 
        //     { 0xF9, SharpSM83::LD }, { 0xFA, SharpSM83::LD }, { 0xFB, SharpSM83::EI },
        // };


    private: //STUFF

        const AddressBus& m_Bus;
        const Decoder& m_Decoder;

        uint8_t m_CyclesToFinish;
    };
}