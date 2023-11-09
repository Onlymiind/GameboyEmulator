#pragma once
#include "gb/cpu/operation.h"
#include "util/util.h"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace gb::cpu {

    enum class Flags : uint8_t {
        CARRY = setBit(4),
        HALF_CARRY = setBit(5),
        NEGATIVE = setBit(6),
        ZERO = setBit(7),

        ALL = CARRY | HALF_CARRY | NEGATIVE | ZERO,

        C = CARRY,
        H = HALF_CARRY,
        N = NEGATIVE,
        Z = ZERO,
    };

    class RegisterFile {
        static constexpr size_t g_flags = 0;

      public:
        uint8_t &A() { return registers_[size_t(Registers::A)]; }
        uint8_t &B() { return registers_[size_t(Registers::B)]; }
        uint8_t &C() { return registers_[size_t(Registers::C)]; }
        uint8_t &D() { return registers_[size_t(Registers::D)]; }
        uint8_t &E() { return registers_[size_t(Registers::E)]; }
        uint8_t &H() { return registers_[size_t(Registers::H)]; }
        uint8_t &L() { return registers_[size_t(Registers::L)]; }

        const uint8_t &A() const { return registers_[size_t(Registers::A)]; }
        const uint8_t &B() const { return registers_[size_t(Registers::B)]; }
        const uint8_t &C() const { return registers_[size_t(Registers::C)]; }
        const uint8_t &D() const { return registers_[size_t(Registers::D)]; }
        const uint8_t &E() const { return registers_[size_t(Registers::E)]; }
        const uint8_t &H() const { return registers_[size_t(Registers::H)]; }
        const uint8_t &L() const { return registers_[size_t(Registers::L)]; }

        void setLow(Registers reg, uint8_t data) {
            size_t idx = uint8_t(reg) & uint8_t(Registers::LOW_REG_MASK);
            if (idx >= registers_.size()) [[unlikely]] {
                throw std::invalid_argument("register index out of bounds");
            }
            registers_[idx] = data;
            registers_[g_flags] &= uint8_t(Flags::ALL);
        }

        void setHigh(Registers reg, uint8_t data) {
            size_t idx = (uint8_t(reg) & uint8_t(Registers::HIGH_REG_MASK)) >> 4;
            if (idx == 0) [[unlikely]] {
                throw std::invalid_argument("attempting to setHigh() of byte register");
            }
            if (idx >= registers_.size()) [[unlikely]] {
                throw std::invalid_argument("register index out of bounds");
            }
            registers_[idx] = data;
        }

        void setAF(uint16_t data) {
            *(std::bit_cast<uint16_t *>(&registers_[g_flags])) = data;
            registers_[g_flags] &= uint8_t(Flags::ALL);
        }
        uint16_t &BC() { return *(std::bit_cast<uint16_t *>(&registers_[size_t(Registers::C)])); }
        uint16_t &DE() { return *(std::bit_cast<uint16_t *>(&registers_[size_t(Registers::E)])); }
        uint16_t &HL() { return *(std::bit_cast<uint16_t *>(&registers_[size_t(Registers::L)])); }

        const uint16_t &AF() const { return *(std::bit_cast<const uint16_t *>(&registers_[g_flags])); }
        const uint16_t &BC() const { return *(std::bit_cast<const uint16_t *>(&registers_[size_t(Registers::C)])); }
        const uint16_t &DE() const { return *(std::bit_cast<const uint16_t *>(&registers_[size_t(Registers::E)])); }
        const uint16_t &HL() const { return *(std::bit_cast<const uint16_t *>(&registers_[size_t(Registers::L)])); }

        void setFlag(Flags flag, bool value) {
            registers_[g_flags] &= ~uint8_t(flag);
            registers_[g_flags] |= value ? uint8_t(flag) : 0;
        }

        bool getFlag(Flags flag) const { return (registers_[g_flags] & uint8_t(flag)) != 0; }

        void clearFlags() { registers_[g_flags] = 0; }

        uint16_t pc;
        uint16_t sp;

      private:
        alignas(uint16_t) std::array<uint8_t, 8> registers_;
    };

    inline bool carried(uint8_t lhs, uint8_t rhs) { return (std::numeric_limits<uint8_t>::max() - rhs) < lhs; }
    inline bool borrowed(uint8_t lhs, uint8_t rhs) { return lhs < rhs; }
    inline bool carried(uint16_t lhs, uint16_t rhs) { return (std::numeric_limits<uint16_t>::max() - rhs) < lhs; }

    inline bool halfCarried(uint8_t lhs, uint8_t rhs) { return ((lhs & 0x0F) + (rhs & 0x0F)) > 0x0F; }
    inline bool halfBorrowed(uint8_t lhs, uint8_t rhs) { return (lhs & 0x0F) < (rhs & 0x0F); }
    inline bool halfCarried(uint16_t rhs, uint16_t lhs) { return ((lhs & 0x0FFF) + (rhs & 0x0FFF)) > 0x0FFF; }
} // namespace gb::cpu
