#pragma once
#include "utils/Utils.h"

#include <cstdint>

namespace gb
{
    namespace cpu
    {
        //TODO: this class
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

            bool carried();
            bool halfCarried();

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

            // WordRegister BC;
            // uint8_t& C = BC.getLow();
            // uint8_t& B = BC.getHight();

            union {
                uint16_t BC;

                struct {
                    uint8_t C;
                    uint8_t B;
                };
            };

            // WordRegister DE;
            // uint8_t& E = DE.getLow();
            // uint8_t& D = DE.getHight();

            union {
                uint16_t DE;

                struct {
                    uint8_t E;
                    uint8_t D;
                };
            };

            // WordRegister HL;
            // uint8_t& L = HL.getLow();
            // uint8_t& H = HL.getHight();
            
            union {
                uint16_t HL;

                struct {
                    uint8_t L;
                    uint8_t H;
                };
            };

            // Stack pointer, program counter
            uint16_t SP;
            uint16_t PC;
        };

        //TODO: is this enough for any instruction?
        struct InstructionContext
        {
            Registers& registers;
            uint16_t additional_data;
        };

        using Instruction = Coroutine<InstructionContext, void>;
    }
}