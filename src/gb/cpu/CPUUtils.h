#pragma once
#include "utils/Utils.h"

#include <array>
#include <cstdint>

namespace gb::cpu {

    struct Registers {
        union {
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
                } flags;

                uint8_t A;
            };
        };

        union {
            uint16_t BC;
            struct {
                uint8_t C;
                uint8_t B;
            };
        };

        union {
            uint16_t DE;
            struct {
                uint8_t E;
                uint8_t D;
            };
        };

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

    #define BIT(x) 1 << x
    enum class Flags : uint8_t {
        CARRY = BIT(4),
        HALF_CARRY = BIT(5),
        NEGATIVE = BIT(6),
        ZERO = BIT(7),

        ALL = CARRY | HALF_CARRY | NEGATIVE | ZERO,

        C = CARRY,
        H = HALF_CARRY,
        N = NEGATIVE,
        Z = ZERO,
    };
    #undef BIT

    class RegisterFile {
        static constexpr size_t flags = 0;
    public:
        uint8_t& A() { return registers_[1]; } uint8_t& B() { return registers_[3]; }
        uint8_t& C() { return registers_[2]; } uint8_t& D() { return registers_[5]; }
        uint8_t& E() { return registers_[4]; } uint8_t& H() { return registers_[7]; }
        uint8_t& L() { return registers_[6]; }

        const uint8_t& A() const { return registers_[1]; } const uint8_t& B() const { return registers_[3]; }
        const uint8_t& C() const { return registers_[2]; } const uint8_t& D() const { return registers_[5]; }
        const uint8_t& E() const { return registers_[4]; } const uint8_t& H() const { return registers_[7]; }
        const uint8_t& L() const { return registers_[6]; }

        void  setAF(uint16_t data) { 
            *(reinterpret_cast<uint16_t*>(&registers_[0])) = data;
            registers_[flags] &= uint8_t(Flags::ALL);
        }
        uint16_t& BC() { return *(reinterpret_cast<uint16_t*>(&registers_[2])); }
        uint16_t& DE() { return *(reinterpret_cast<uint16_t*>(&registers_[4])); }
        uint16_t& HL() { return *(reinterpret_cast<uint16_t*>(&registers_[6])); }

        const uint16_t& AF() const { return *(reinterpret_cast<const uint16_t*>(&registers_[0])); }
        const uint16_t& BC() const { return *(reinterpret_cast<const uint16_t*>(&registers_[2])); }
        const uint16_t& DE() const { return *(reinterpret_cast<const uint16_t*>(&registers_[4])); }
        const uint16_t& HL() const { return *(reinterpret_cast<const uint16_t*>(&registers_[6])); }

        void setFlag(Flags flag, bool value) {
            registers_[flags] &= ~uint8_t(flag);
            registers_[flags] |= value ? uint8_t(flag) : 0;
        }

        bool getFlag(Flags flag) const { return (registers_[flags] & uint8_t(flag)) != 0; }

        void clearFlags() { registers_[flags] = 0; }

        uint16_t PC;
        uint16_t SP;
    private:
        alignas(uint16_t) std::array<uint8_t, 8> registers_;
    };

    inline bool carried(uint8_t lhs, uint8_t rhs) { return (std::numeric_limits<uint8_t>::max() - rhs) < lhs; }
    inline bool borrowed(uint8_t lhs, uint8_t rhs) { return lhs < rhs; }
    inline bool carried(uint16_t lhs, uint16_t rhs) { return (std::numeric_limits<uint16_t>::max() - rhs) < lhs; }

    inline bool halfCarried(uint8_t lhs, uint8_t rhs) { return ((lhs & 0x0F) + (rhs & 0x0F)) > 0x0F; }
    inline bool halfBorrowed(uint8_t lhs, uint8_t rhs) { return (lhs & 0x0F) < (rhs & 0x0F); }
    inline bool halfCarried(uint16_t rhs, uint16_t lhs) { return ((lhs & 0x0FFF) + (rhs & 0x0FFF)) > 0x0FFF; }
}
