#ifndef GB_EMULATOR_SRC_GB_ADDRESS_BUS_HDR_
#define GB_EMULATOR_SRC_GB_ADDRESS_BUS_HDR_

#include "gb/gb_input.h"
#include "gb/interrupt_register.h"
#include "gb/memory/basic_components.h"
#include "gb/memory/memory_map.h"
#include "gb/ppu/ppu.h"
#include "gb/timer.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace gb {
    class IMemoryObserver {
      public:
        virtual void onRead(uint16_t address, uint8_t data) noexcept {};
        virtual void onWrite(uint16_t address, uint8_t data) noexcept {};
        virtual uint16_t minAddress() const noexcept = 0;
        virtual uint16_t maxAddress() const noexcept = 0;

        bool isInRange(uint16_t address) const { return address >= minAddress() && address <= maxAddress(); }

      protected:
        ~IMemoryObserver() = default;
    };

    constexpr std::array<uint8_t, 77> g_io_initail_values = {0xcf, 0,    0x7e, 0xff, 0x19, 0,    0,    0xf8, 0xff, 0xff,
                                                             0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0x80, 0xbf, 0xf3, 0xff,
                                                             0xbf, 0xff, 0x3f, 0,    0xff, 0xbf, 0x7f, 0xff, 0x9f, 0xff,
                                                             0xbf, 0xff, 0xff, 0,    0,    0xbf, 0x77, 0xf3, 0xf1, 0xff,
                                                             0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0,
                                                             0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
                                                             0,    0,    0,    0,    0,    0x91, 0x83, 0,    0,    1,
                                                             0,    0xff, 0xfc, 0xff, 0xff, 0,    0};

    class AddressBus {
      public:
        AddressBus(WRAM wram, UnusedIO unused_io, HRAM hram, Cartridge &cartridge, PPU &ppu, Timer &timer, Input &input,
                   InterruptRegister &interrupt_enable, InterruptRegister &interrupt_flags)
            : wram_(wram), unused_io_(unused_io), hram_(hram), cartridge_(cartridge),
              interrupt_enable_(interrupt_enable), interrupt_flags_(interrupt_flags), timer_(timer), ppu_(ppu),
              input_(input) {}

        void setObserver(IMemoryObserver &observer) { observer_ = &observer; }
        void removeObserver() { observer_ = nullptr; }

        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data);

        void reset();

        std::optional<uint8_t> peek(uint16_t address) const;

      private:
        std::string getErrorDescription(uint16_t address, int value = -1) const;

        IMemoryObserver *observer_ = nullptr;

        WRAM wram_;
        UnusedIO unused_io_;
        HRAM hram_;

        Cartridge &cartridge_;
        InterruptRegister &interrupt_enable_;
        InterruptRegister &interrupt_flags_;
        Timer &timer_;
        PPU &ppu_;
        Input &input_;

        uint8_t data_ = 0;
    };

} // namespace gb

#endif
