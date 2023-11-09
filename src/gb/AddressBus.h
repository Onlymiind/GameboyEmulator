#pragma once
#include "gb/InterruptRegister.h"
#include "gb/Timer.h"
#include "gb/memory/BasicComponents.h"
#include "gb/ppu/PPU.h"

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

    constexpr MemoryObjectInfo g_memory_rom = {.min_address = 0x0000, .max_address = 0x7FFF};
    constexpr MemoryObjectInfo g_memory_cartridge_ram = {.min_address = 0xa000, .max_address = 0xbfff};
    constexpr MemoryObjectInfo g_memory_wram = {.min_address = 0xc000, .max_address = 0xdfff};
    constexpr MemoryObjectInfo g_memory_mirror = {.min_address = 0xe000, .max_address = 0xfdff};
    constexpr MemoryObjectInfo g_memory_forbidden = {.min_address = 0xfea0, .max_address = 0xfeff};
    constexpr MemoryObjectInfo g_memory_timer = {.min_address = 0xFF04, .max_address = 0xFF07};
    constexpr MemoryObjectInfo g_memory_hram = {.min_address = 0xff80, .max_address = 0xfffe};

    // FIXME: this should eventually be properly mapped
    constexpr MemoryObjectInfo g_memory_io_unused = {.min_address = 0xff00, .max_address = 0xff7f};

    enum class MemoryObjectType { ROM, VRAM, CARTRIDGE_RAM, WRAM, OAM, IO, HRAM, IE };

    constexpr MemoryObjectInfo objectTypeToInfo(MemoryObjectType type) {
        using enum MemoryObjectType;
        switch (type) {
        case ROM: return g_memory_rom;
        case VRAM: return g_memory_vram;
        case CARTRIDGE_RAM: return g_memory_cartridge_ram;
        case WRAM: return g_memory_wram;
        case OAM: return g_memory_oam;
        case IO: return g_memory_io_unused;
        case HRAM: return g_memory_hram;
        case IE: return MemoryObjectInfo{g_interrupt_enable_address, g_interrupt_enable_address};
        default: return MemoryObjectInfo{};
        }
    }

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
        AddressBus(Cartridge &cartridge, PPU &ppu, Timer &timer, InterruptRegister &interrupt_enable,
                   InterruptRegister &interrupt_flags)
            : cartridge_(cartridge), interrupt_enable_(interrupt_enable), interrupt_flags_(interrupt_flags),
              timer_(timer), ppu_(ppu) {}

        void setObserver(IMemoryObserver &observer) { observer_ = &observer; }
        void removeObserver() { observer_ = nullptr; }

        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data);

        void reset();

        std::optional<uint8_t> peek(uint16_t address) const;

      private:
        std::string getErrorDescription(uint16_t address, int value = -1) const;

        IMemoryObserver *observer_ = nullptr;

        RAM<g_memory_vram.size> vram_{};
        RAM<g_memory_wram.size> wram_{};
        RAM<g_memory_oam.size> oam_{};

        RAM<g_memory_io_unused.size> unused_io_{};

        RAM<g_memory_hram.size> hram_{};
        Cartridge &cartridge_;
        InterruptRegister &interrupt_enable_;
        InterruptRegister &interrupt_flags_;
        Timer &timer_;
        PPU &ppu_;

        uint8_t data_ = 0;
    };

} // namespace gb
