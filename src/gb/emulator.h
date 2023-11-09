#pragma once
#include "gb/address_bus.h"
#include "gb/cpu/cpu.h"
#include "gb/cpu/cpu_utils.h"
#include "gb/cpu/decoder.h"
#include "gb/interrupt_register.h"
#include "gb/memory/basic_components.h"
#include "gb/ppu/ppu.h"
#include "gb/timer.h"
#include "util/util.h"

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

        void reset() {
            cpu_.reset();
            bus_.reset();
            ppu_.reset();
            timer_.reset();
            ie_.write(0);
            if_.setFlag(InterruptFlags::VBLANK);
        }

        void start() {
            is_running_ = true;
            try {
                // fetch the first instruction
                cpu_.tick();
            } catch (...) {
                is_running_ = false;
                throw;
            }
        }

        void stop() { is_running_ = false; }

        std::optional<uint8_t> peekMemory(uint16_t address) { return bus_.peek(address); }

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
                timer_.update();
                ppu_.update();
            }
            is_running_ = !cpu_.isStopped();
        } catch (...) {
            is_running_ = false;
            throw;
        }
    }
} // namespace gb