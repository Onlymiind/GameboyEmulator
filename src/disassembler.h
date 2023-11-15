#ifndef GB_EMULATOR_SRC_DISASSEMBLER_HDR_
#define GB_EMULATOR_SRC_DISASSEMBLER_HDR_

#include "gb/cpu/cpu.h"
#include <cstdint>
#include <map>
namespace emulator {
    struct InstructionAddress {
        static constexpr uint16_t g_none_bank = uint16_t(-1);

        uint16_t address = 0;

        // according to docs, 512 is the maximum possible amount of banks in a valid cartridge
        uint16_t bank = g_none_bank;
    };

    constexpr inline bool operator<(InstructionAddress lhs, InstructionAddress rhs) {
        if (lhs.bank == InstructionAddress::g_none_bank || rhs.bank == InstructionAddress::g_none_bank) {
            return lhs.address < rhs.address;
        }

        if (lhs.bank == rhs.bank) {
            return lhs.address < rhs.address;
        }

        return lhs.bank < rhs.bank;
    }

    constexpr inline bool operator==(InstructionAddress lhs, InstructionAddress rhs) {
        return lhs.address == rhs.address && lhs.bank == rhs.bank;
    }

    // TODO: this doesn't support self-modifyng code
    class Disassembler {
      public:
        using Iterator = std::map<InstructionAddress, gb::cpu::Instruction>::iterator;
        using ConstInerator = std::map<InstructionAddress, gb::cpu::Instruction>::const_iterator;

        Disassembler() = default;

        void addInstruction(gb::cpu::Instruction instr, uint16_t bank = InstructionAddress::g_none_bank) {
            gb::cpu::Instruction &value = disassembly_[InstructionAddress{
                .address = instr.registers.pc(),
                .bank = bank,
            }];
            if (value != instr) {
                dirty_ = true;
            }
            value = instr;
        }

        Iterator begin() { return disassembly_.begin(); }
        Iterator end() { return disassembly_.end(); }
        Iterator at(InstructionAddress addr) { return disassembly_.find(addr); }

        ConstInerator begin() const { return disassembly_.begin(); }
        ConstInerator end() const { return disassembly_.end(); }
        ConstInerator at(InstructionAddress addr) const { return disassembly_.find(addr); }

        size_t size() const { return disassembly_.size(); }

        void clear() {
            dirty_ = true;
            disassembly_.clear();
        }

        bool isDirty() const { return dirty_; }
        void clearDirtyFlag() { dirty_ = false; }

      private:
        std::map<InstructionAddress, gb::cpu::Instruction> disassembly_;
        bool dirty_ = true;
    };
} // namespace emulator

#endif
