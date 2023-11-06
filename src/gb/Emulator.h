#pragma once
#include "gb/AddressBus.h"
#include "gb/InterruptRegister.h"
#include "gb/Timer.h"
#include "gb/cpu/CPU.h"
#include "gb/cpu/CPUUtils.h"
#include "gb/cpu/Decoder.h"
#include "gb/memory/BasicComponents.h"
#include "gb/ppu/PPU.h"
#include "utils/Utils.h"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace gb {

    class Emulator {
      public:
        Emulator() = default;

        void tick();

        bool terminated() const { return !is_running_; }

        void reset() { cpu_.reset(); }

        void start() { is_running_ = true; }

        void stop() { is_running_ = false; }

        uint8_t peekMemory(uint16_t address) { return bus_.read(address); }

        cpu::SharpSM83 &getCPU() { return cpu_; }
        AddressBus &getBus() { return bus_; }
        Timer &getTimer() { return timer_; }
        Cartridge &getCartridge() { return cartridge_; }
        PPU &getPPU() { return ppu_; }
        InterruptRegister &getIE() { return ie_; }
        InterruptRegister &getIF() { return if_; }

      private:
        Cartridge cartridge_;
        InterruptRegister ie_;
        InterruptRegister if_;
        Timer timer_{if_};
        PPU ppu_{if_};
        AddressBus bus_{cartridge_, ppu_, timer_, ie_, if_};
        cpu::SharpSM83 cpu_{bus_, ie_, if_};

        bool is_running_ = false;
    };

    inline void Emulator::tick() {
        if (!is_running_) {
            return;
        }

        try {
            cpu_.tick();
            for (int i = 0; i < 4; ++i) {
                bus_.update();
            }
            is_running_ = !cpu_.isStopped();
        } catch (...) {
            is_running_ = false;
            throw;
        }
    }
} // namespace gb
