#pragma once
#include "utils/Utils.h"

#include <cstdint>

namespace gb
{
    namespace cpu
    {
        class WordRegister
        {
        public:
            WordRegister()
            {}
            WordRegister(uint16_t value)
                : high_((value & 0xFF00) >> 8), low_(value & 0x00FF)
            {}

            uint8_t& getHight();
            uint8_t& getLow();

            uint16_t value() const;

            //TODO
            // bool carried();
            // bool halfCarried();

            WordRegister& operator+=(uint16_t value);
            WordRegister& operator-=(uint16_t value);
            WordRegister& operator=(uint16_t value);

            WordRegister operator--();
            WordRegister operator++();

            explicit operator bool();

            operator uint16_t() const;
        private:
            uint16_t pack() const;

            void unpack(uint16_t value);
        private:
            uint8_t high_;
            uint8_t low_;
        };

        struct Registers
        {
            union 
            {
                uint16_t AF;
                
                struct {
                    union {
                        uint8_t value;

                        struct {
                            uint8_t unused : 4;
                            uint8_t C : 1;
                            uint8_t H : 1;
                            uint8_t N : 1;
                            uint8_t Z : 1;
                        };
                    }flags;

                    uint8_t A;
                };
            };

            WordRegister BC;
            uint8_t& C = BC.getLow();
            uint8_t& B = BC.getHight();

            WordRegister DE;
            uint8_t& E = DE.getLow();
            uint8_t& D = DE.getHight();

            WordRegister HL;
            uint8_t& L = HL.getLow();
            uint8_t& H = HL.getHight();

            // Stack pointer, program counter
            uint16_t SP;
            uint16_t PC;
        };

        //TODO: is this enough for any instruction?
        struct InstructionContext
        {
            Registers* registers = nullptr;
            //Variable to hold address/immediate value
            uint16_t arg = 0;
            //Variable to hold intermediate result
            uint16_t value = 0;
        };

        using Instruction = Coroutine<InstructionContext, void>;
    }
}