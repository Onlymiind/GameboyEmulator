#pragma once
#include "utils/Utils.h"

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
        ZERO = BIT(7)
    };
    #undef BIT

    inline bool carried(uint8_t lhs, uint8_t rhs) { return (std::numeric_limits<uint8_t>::max() - rhs) < lhs; }
    inline bool borrowed(uint8_t lhs, uint8_t rhs) { return lhs < rhs; }
    inline bool carried(uint16_t lhs, uint16_t rhs) { return (std::numeric_limits<uint16_t>::max() - rhs) < lhs; }

    inline bool halfCarried(uint8_t lhs, uint8_t rhs) { return ((lhs & 0x0F) + (rhs & 0x0F)) > 0x0F; }
    inline bool halfBorrowed(uint8_t lhs, uint8_t rhs) { return (lhs & 0x0F) < (rhs & 0x0F); }
    inline bool halfCarried(uint16_t rhs, uint16_t lhs) { return ((lhs & 0x0FFF) + (rhs & 0x0FFF)) > 0x0FFF; }
}
