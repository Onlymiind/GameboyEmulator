#pragma once
#include "gb/cpu/CPUUtils.h"
#include "gb/memory/BasicComponents.h"
#include "gb/AddressBus.h"
#include "gb/cpu/CPU.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"
#include "gb/Timer.h"
#include "gb/memory/Memory.h"

#include <cstdint>
#include <vector>

namespace gb {

    struct MemoryObjectInfo {
        uint16_t min_address, max_address = 0;
        uint16_t size = static_cast<uint16_t>(max_address - min_address + 1);
    };

    constexpr MemoryObjectInfo g_ROM = {0x0000, 0x7FFF};
    constexpr MemoryObjectInfo g_RAM = {0x8000, 0xFF03};
    constexpr MemoryObjectInfo g_timer = {0xFF04, 0xFF07};
    constexpr MemoryObjectInfo g_leftover2 = {0xFF08, 0xFF0E};
    constexpr MemoryObjectInfo g_interrupt_enable = {0xFF0F, 0xFF0F};
    constexpr MemoryObjectInfo g_leftover = {0xFF10, 0xFFFE};
    constexpr MemoryObjectInfo g_interrupt_flags = {0xFFFF, 0xFFFF};

    class Emulator {
    public:
        Emulator();

        cpu::Registers getRegisters() const { return cpu_.getRegisters(); }

        uint16_t getPC() const { return cpu_.getProgramCounter(); }

        void addMemoryObserver(MemoryController observer) { bus_.addObserver(observer); }

        void tick();

        bool terminated() const;

        void reset() { cpu_.reset(); }

        void setROM(std::vector<uint8_t> rom) { ROM_.setData(std::move(rom)); }

        void start() { is_running_ = true; }

    private:
        RAM RAM_;
        ROM ROM_;
        RAM leftover_;
        RAM leftover2_;
        AddressBus bus_;
        InterruptRegister interrupt_enable_;
        InterruptRegister interrupt_flags_;
        decoding::Decoder decoder_;
        cpu::SharpSM83 cpu_{bus_, interrupt_enable_, interrupt_flags_, decoder_};
        Timer timer_{interrupt_flags_};

        bool is_running_ = false;
    };

    inline Emulator::Emulator()
        :RAM_(g_RAM.size), leftover_(g_leftover.size), leftover2_(g_leftover2.size)
    {
        bus_.connect({g_ROM.min_address, g_ROM.max_address, ROM_});
        bus_.connect({g_RAM.min_address, g_RAM.max_address, RAM_});
        bus_.connect({g_timer.min_address, g_timer.max_address, timer_});
        bus_.connect({g_leftover2.min_address, g_leftover2.max_address, leftover2_});
        bus_.connect({g_interrupt_enable.min_address, g_interrupt_enable.max_address, interrupt_enable_});
        bus_.connect({g_leftover.min_address, g_leftover.max_address, leftover_});
        bus_.connect({g_interrupt_flags.min_address, g_interrupt_flags.max_address, interrupt_flags_});
    }
}
