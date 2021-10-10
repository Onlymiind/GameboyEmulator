#pragma once
#include "utils/Utils.h"

#include <cstdint>
#include <array>

namespace gb
{
    namespace cpu
    {
        class Word
        {
        public:
            Word()
            {}
            Word(uint16_t value)
                : high_((value & 0xFF00) >> 8), low_(value & 0x00FF)
            {}

            uint8_t& getHight();
            uint8_t& getLow();

            uint16_t value() const;

            Word& operator+=(uint16_t value);
            Word& operator-=(uint16_t value);
            Word& operator=(uint16_t value);

            Word operator--();
            Word operator++();

            explicit operator bool();
        private:
            uint16_t pack() const;

            void unpack(uint16_t value);
        private:
            uint8_t high_;
            uint8_t low_;
        };

        bool carried(uint8_t lhs, uint8_t rhs);
        bool borrowed(uint8_t lhs, uint8_t rhs); 
        bool carried(uint16_t lhs, uint16_t rhs);

        bool halfCarried(uint8_t lhs, uint8_t rhs);
        bool halfBorrowed(uint8_t lhs, uint8_t rhs);
        bool halfCarried(uint16_t rhs, uint16_t lhs);

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

            Word BC;
            uint8_t& C = BC.getLow();
            uint8_t& B = BC.getHight();

            Word DE;
            uint8_t& E = DE.getLow();
            uint8_t& D = DE.getHight();

            Word HL;
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
            
            //Stores all kinds of intermediate results, e.g. values of registers or addresses
            std::array<uint16_t, 3> storage;
            //Some common indexes for more readable code
            const int value_indx = 0;
            const int addr_indx = 1;
        };

        using Instruction = Coroutine<InstructionContext>;
    }
}