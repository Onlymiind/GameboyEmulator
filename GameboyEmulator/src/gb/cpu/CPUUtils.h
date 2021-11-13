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
            Word(uint8_t& high, uint8_t& low)
                : high_(high), low_(low)
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
            uint8_t& high_;
            uint8_t& low_;
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

        enum class ByteRegisters
        {
            A, B, C, D, E, H, L,

            COUNT
        };

        enum class Flags
        {
            Carry     = 4,
            HalfCarry = 5,
            Negative  = 6,
            Zero      = 7
        };

        enum class WordRegisters
        {
            AF, BC, DE, HL, SP,

            COUNT
        };

        class RegisterFile
        {
        public:
            RegisterFile() = default;

            void setFlag(Flags flag, bool value)
            {
                uint8_t flag_mask = static_cast<uint8_t>(value) << static_cast<uint8_t>(flag);
                byte_registers_[flags_index_] = (byte_registers_[flags_index_] & (~flag_mask)) | flag_mask;
            }

            bool getFlag(Flags flag)
            {
                return (byte_registers_[flags_index_] & (1 << static_cast<uint8_t>(flag))) != 0;
            }

            uint8_t& operator[](ByteRegisters reg)
            {
                return byte_registers_[static_cast<int>(reg)];
            }

            Word& operator[](WordRegisters reg)
            {
                return word_regisrers_[static_cast<int>(reg)];
            }
        private:
            const size_t flags_index_ = 7;
            // +3 to compensate for F and SP registers
            // Layout: A, B, C, D, E, H, L, F, SP
            std::array<uint8_t, static_cast<int>(ByteRegisters::COUNT) + 3> byte_registers_;
            std::array<Word, static_cast<int>(WordRegisters::COUNT)> word_regisrers_ =
            {
                Word(byte_registers_[0], byte_registers_[flags_index_]), //AF
                Word(byte_registers_[1], byte_registers_[2]),            //BC
                Word(byte_registers_[3], byte_registers_[4]),            //DE
                Word(byte_registers_[5], byte_registers_[6]),            //HL
                Word(byte_registers_[8], byte_registers_[9])             //SP
            };
        };
    }
}