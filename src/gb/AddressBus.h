#pragma once
#include "gb/InterruptRegister.h"
#include "gb/Timer.h"
#include "gb/memory/BasicComponents.h"
#include "gb/ppu/PPU.h"

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
    constexpr MemoryObjectInfo g_memory_io_unused = {.min_address = 0xff00, .max_address = 0xff03};
    constexpr MemoryObjectInfo g_memory_timer = {.min_address = 0xFF04, .max_address = 0xFF07};
    constexpr MemoryObjectInfo g_memory_io_unused2 = {.min_address = 0xff08, .max_address = 0xff0e};
    constexpr MemoryObjectInfo g_memory_io_unused3 = {.min_address = 0xff10, .max_address = 0xff7f};
    constexpr MemoryObjectInfo g_memory_hram = {.min_address = 0xff80, .max_address = 0xfffe};

    enum class MemoryObjectType { ROM, VRAM, CARTRIDGE_RAM, WRAM, OAM, IO, HRAM, IE };

    constexpr MemoryObjectInfo objectTypeToInfo(MemoryObjectType type) {
        using enum MemoryObjectType;
        switch (type) {
        case ROM: return g_memory_rom;
        case VRAM: return g_memory_vram;
        case CARTRIDGE_RAM: return g_memory_cartridge_ram;
        case WRAM: return g_memory_wram;
        case OAM: return g_memory_oam;
        case IO:
            return MemoryObjectInfo{.min_address = g_memory_io_unused.min_address,
                                    .max_address = g_memory_io_unused3.max_address};
        case HRAM: return g_memory_hram;
        case IE: return MemoryObjectInfo{g_interrupt_enable_address, g_interrupt_enable_address};
        default: return MemoryObjectInfo{};
        }
    }

    class AddressBus {
      public:
        AddressBus(Cartridge &cartridge, PPU &ppu, Timer &timer, InterruptRegister &interrupt_enable,
                   InterruptRegister &interrupt_flags)
            : cartridge_(cartridge), interrupt_enable_(interrupt_enable), interrupt_flags_(interrupt_flags),
              timer_(timer), ppu_(ppu) {}

        void setROMData(std::vector<uint8_t> data) { cartridge_.setROM(std::move(data)); }
        void update() { timer_.update(); }

        void setObserver(IMemoryObserver &observer) { observer_ = &observer; }
        void removeObserver() { observer_ = nullptr; }

        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data);

      private:
        std::string getErrorDescription(uint16_t address, int value = -1) const;

        IMemoryObserver *observer_ = nullptr;

        RAM<g_memory_vram.size> vram_{};
        RAM<g_memory_wram.size> wram_{};
        RAM<g_memory_oam.size> oam_{};

        RAM<g_memory_io_unused.size> unused_io_{};
        RAM<g_memory_io_unused2.size> unused_io2_{};
        RAM<g_memory_io_unused3.size> unused_io3_{};

        RAM<g_memory_hram.size> hram_{};
        Cartridge &cartridge_;
        InterruptRegister &interrupt_enable_;
        InterruptRegister &interrupt_flags_;
        Timer &timer_;
        PPU &ppu_;

        uint8_t data_ = 0;
    };

} // namespace gb
