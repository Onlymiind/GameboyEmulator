#ifndef GB_EMULATOR_SRC_GB_CPU_CPU_UTILS_HDR_
#define GB_EMULATOR_SRC_GB_CPU_CPU_UTILS_HDR_

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

        uint8_t A() const { return registers_[size_t(Registers::A)]; }
        uint8_t B() const { return registers_[size_t(Registers::B)]; }
        uint8_t C() const { return registers_[size_t(Registers::C)]; }
        uint8_t D() const { return registers_[size_t(Registers::D)]; }
        uint8_t E() const { return registers_[size_t(Registers::E)]; }
        uint8_t H() const { return registers_[size_t(Registers::H)]; }
        uint8_t L() const { return registers_[size_t(Registers::L)]; }

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

        uint8_t getByteRegister(Registers reg) const {
            size_t idx = reg & Registers::LOW_REG_MASK;
            if (idx >= registers_.size()) [[unlikely]] {
                throw std::invalid_argument("invalid byte register");
            }
            return registers_[idx];
        }
        uint16_t getWordRegister(Registers reg) const {
            size_t idx = reg & Registers::LOW_REG_MASK;
            if (isByteRegister(reg) || idx >= registers_.size()) [[unlikely]] {
                throw std::invalid_argument("invalid word register");
            }

            return *std::bit_cast<uint16_t *>(&registers_[idx]);
        }

        void setWordRegister(Registers reg, uint16_t data) {
            setLow(reg, uint8_t(data));
            setHigh(reg, uint8_t(data >> 8));
        }

        uint16_t AF() const { return uint16_t(registers_[g_flags]) | (uint16_t(A()) << 8); }
        uint16_t BC() const { return uint16_t(C()) | (uint16_t(B()) << 8); }
        uint16_t DE() const { return uint16_t(E()) | (uint16_t(D()) << 8); }
        uint16_t HL() const { return uint16_t(L()) | (uint16_t(H()) << 8); }
        uint16_t PC() const {
            return uint16_t(registers_[size_t(Registers::PC_LOW)]) |
                   (uint16_t(registers_[size_t(Registers::PC_HIGH)]) << 8);
        }

        void AF(uint16_t value) {
            registers_[g_flags] = uint8_t(value) & uint8_t(Flags::ALL);
            registers_[size_t(Registers::A)] = uint8_t(value >> 8);
        }
        void BC(uint16_t value) {
            registers_[size_t(Registers::C)] = uint8_t(value);
            registers_[size_t(Registers::B)] = uint8_t(value >> 8);
        }

        void DE(uint16_t value) {
            registers_[size_t(Registers::E)] = uint8_t(value);
            registers_[size_t(Registers::D)] = uint8_t(value >> 8);
        }

        void HL(uint16_t value) {
            registers_[size_t(Registers::L)] = uint8_t(value);
            registers_[size_t(Registers::H)] = uint8_t(value >> 8);
        }

        void PC(uint16_t value) {
            registers_[size_t(Registers::PC_LOW)] = uint8_t(value);
            registers_[size_t(Registers::PC_HIGH)] = uint8_t(value >> 8);
        }

        void setFlag(Flags flag, bool value) {
            registers_[g_flags] &= ~uint8_t(flag);
            registers_[g_flags] |= value ? uint8_t(flag) : 0;
        }

        bool getFlag(Flags flag) const { return (registers_[g_flags] & uint8_t(flag)) != 0; }

        void clearFlags() { registers_[g_flags] = 0; }

        uint16_t sp;

      private:
        alignas(uint16_t) std::array<uint8_t, 10> registers_; // 7 registers + flags + PC
    };

    inline bool carried(uint8_t lhs, uint8_t rhs) { return (std::numeric_limits<uint8_t>::max() - rhs) < lhs; }
    inline bool borrowed(uint8_t lhs, uint8_t rhs) { return lhs < rhs; }
    inline bool carried(uint16_t lhs, uint16_t rhs) { return (std::numeric_limits<uint16_t>::max() - rhs) < lhs; }

    inline bool halfCarried(uint8_t lhs, uint8_t rhs) { return ((lhs & 0x0F) + (rhs & 0x0F)) > 0x0F; }
    inline bool halfBorrowed(uint8_t lhs, uint8_t rhs) { return (lhs & 0x0F) < (rhs & 0x0F); }
    inline bool halfCarried(uint16_t rhs, uint16_t lhs) { return ((lhs & 0x0FFF) + (rhs & 0x0FFF)) > 0x0FFF; }
} // namespace gb::cpu

#endif
