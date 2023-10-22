#pragma once
#include "gb/InterruptRegister.h"
#include "gb/Timer.h"
#include "gb/memory/BasicComponents.h"

#include <cstdint>
#include <string>
#include <vector>

namespace gb {
    class MemoryObserver {
      public:
        virtual void onRead(uint16_t address, uint8_t data){};
        virtual void onWrite(uint16_t address, uint8_t data){};
        virtual uint16_t minAddress() const = 0;
        virtual uint16_t maxAddress() const = 0;

        bool isInRange(uint16_t address) const {
            return address >= minAddress() && address <= maxAddress();
        }
    };

    constexpr MemoryObjectInfo g_memory_rom = {.min_address = 0x0000, .max_address = 0x7FFF};
    constexpr MemoryObjectInfo g_memory_vram = {.min_address = 0x8000, .max_address = 0x9fff};
    constexpr MemoryObjectInfo g_memory_cartridge_ram = {.min_address = 0xa000,
                                                         .max_address = 0xbfff};
    constexpr MemoryObjectInfo g_memory_wram = {.min_address = 0xc000, .max_address = 0xdfff};
    constexpr MemoryObjectInfo g_memory_mirror = {.min_address = 0xe000, .max_address = 0xfdff};
    constexpr MemoryObjectInfo g_memory_oam = {.min_address = 0xe000, .max_address = 0xfe9f};
    constexpr MemoryObjectInfo g_memory_forbidden = {.min_address = 0xfea0, .max_address = 0xfeff};
    constexpr MemoryObjectInfo g_memory_io_unused = {.min_address = 0xff00, .max_address = 0xff03};
    constexpr MemoryObjectInfo g_memory_timer = {.min_address = 0xFF04, .max_address = 0xFF07};
    constexpr MemoryObjectInfo g_memory_io_unused2 = {.min_address = 0xff08, .max_address = 0xff0e};
    constexpr MemoryObjectInfo g_memory_io_unused3 = {.min_address = 0xff10, .max_address = 0xff7f};
    constexpr MemoryObjectInfo g_memory_hram = {.min_address = 0xff80, .max_address = 0xfffe};

    class AddressBus {
      public:
        AddressBus() {}

        ~AddressBus() = default;

        void setRomData(std::vector<uint8_t> data) { cartridge_.setROM(std::move(data)); }
        void update() { timer_.update(); }

        void setObserver(MemoryObserver &observer) { observer_ = &observer; }
        void removeObserver() { observer_ = nullptr; }

        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data);

        InterruptRegister &getInterruptFlags() { return interrupt_flags_; }
        InterruptRegister &getInterruptEnable() { return interrupt_enable_; }

        const InterruptRegister &getInterruptFlags() const { return interrupt_flags_; }
        const InterruptRegister &getInterruptEnable() const { return interrupt_enable_; }

      private:
        std::string getErrorDescription(uint16_t address, int value = -1) const;

        MemoryObserver *observer_ = nullptr;

        RAM<g_memory_vram.size> vram_;
        RAM<g_memory_wram.size> wram_;
        RAM<g_memory_oam.size> oam_;

        RAM<g_memory_io_unused.size> unused_io_;
        RAM<g_memory_io_unused2.size> unused_io2_;
        RAM<g_memory_io_unused3.size> unused_io3_;

        RAM<g_memory_hram.size> hram_;
        Cartridge cartridge_;
        InterruptRegister interrupt_enable_;
        InterruptRegister interrupt_flags_;
        Timer timer_{interrupt_flags_};

        uint8_t data_ = 0;
    };

} // namespace gb
