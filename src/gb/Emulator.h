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

    constexpr MemoryObjectInfo g_memory_rom = {0x0000, 0x7FFF};
    constexpr MemoryObjectInfo g_memory_ram = {0x8000, 0xFF03};
    constexpr MemoryObjectInfo g_memory_timer = {0xFF04, 0xFF07};
    constexpr MemoryObjectInfo g_memory_leftover2 = {0xFF08, 0xFF0E};
    constexpr MemoryObjectInfo g_memory_interrupt_enable = {0xFF0F, 0xFF0F};
    constexpr MemoryObjectInfo g_memory_leftover = {0xFF10, 0xFFFE};
    constexpr MemoryObjectInfo g_memory_interrupt_flags = {0xFFFF, 0xFFFF};

    class Emulator {
    public:
        Emulator();

        cpu::Registers getRegisters() const { return cpu_.getRegisters(); }

        uint16_t getPC() const { return cpu_.getProgramCounter(); }

        void addMemoryObserver(MemoryController observer) { bus_.addObserver(observer); }

        void tick();

        bool terminated() const { return !is_running_; }

        void reset() { cpu_.reset(); }

        void setROM(std::vector<uint8_t> rom) { 
            rom_.setData(std::move(rom)); 
        }

        void start() { is_running_ = true; }

    private:
        RAM ram_;
        ROM rom_;
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
        :ram_(g_memory_ram.size), leftover_(g_memory_leftover.size), leftover2_(g_memory_leftover2.size)
    {
        bus_.connect({g_memory_rom.min_address, g_memory_rom.max_address, rom_});
        bus_.connect({g_memory_ram.min_address, g_memory_ram.max_address, ram_});
        bus_.connect({g_memory_timer.min_address, g_memory_timer.max_address, timer_});
        bus_.connect({g_memory_leftover2.min_address, g_memory_leftover2.max_address, leftover2_});
        bus_.connect({g_memory_interrupt_enable.min_address, g_memory_interrupt_enable.max_address, interrupt_enable_});
        bus_.connect({g_memory_leftover.min_address, g_memory_leftover.max_address, leftover_});
        bus_.connect({g_memory_interrupt_flags.min_address, g_memory_interrupt_flags.max_address, interrupt_flags_});
    }

    inline void Emulator::tick() {
        if(!is_running_) {
            return;
        }
        static uint16_t oldPC = cpu_.getProgramCounter();

        try {
            cpu_.tick();
            for(int i = 0; i < 4; ++i) {
                timer_.update();
            }
        }
        catch(const std::exception& e) {
            is_running_ = false;
            return;
        }

        if(cpu_.isFinished()) {
            if(oldPC == cpu_.getProgramCounter()) {
                is_running_ = false;
            }

            oldPC = cpu_.getProgramCounter();
        }
    }
}
