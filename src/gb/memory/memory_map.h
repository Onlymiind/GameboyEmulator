#ifndef GB_EMULATOR_SRC_GB_MEMORY_MEMORY_MAP_HDR_
#define GB_EMULATOR_SRC_GB_MEMORY_MEMORY_MAP_HDR_

#include "gb/memory/basic_components.h"

#include <cstdint>
#include <span>

namespace gb {
    enum class IO : uint16_t {
        // Joypad
        JOYPAD = 0xff00,

        // Timer
        DIV = 0xff04,
        TIMA = 0xff05,
        TMA = 0xff06,
        TAC = 0xff07,

        // Interrupt flags
        IF = 0xff0f,

        // PPU
        LCDC = 0xff40,
        LCD_STATUS = 0xff41,
        SCROLL_Y = 0xff42,
        SCROLL_X = 0xff43,
        LCD_Y = 0xff44,
        LYC = 0xff45,
        DMA_SRC = 0xff46,
        BG_PALETTE = 0xff47,
        OBJ0_PALETTE = 0xff48,
        OBJ1_PALETTE = 0xff49,
        WINDOW_Y = 0xff4a,
        WINDOW_X = 0xff4b
    };

    constexpr MemoryObjectInfo g_memory_wram = {.min_address = 0xc000, .max_address = 0xdfff};
    constexpr MemoryObjectInfo g_memory_hram = {.min_address = 0xff80, .max_address = 0xfffe};
    // FIXME: this should eventually be properly mapped
    constexpr MemoryObjectInfo g_memory_io_unused = {.min_address = 0xff00, .max_address = 0xff7f};
    constexpr MemoryObjectInfo g_memory_vram = {.min_address = 0x8000, .max_address = 0x9fff};
    constexpr MemoryObjectInfo g_memory_oam = {.min_address = 0xe000, .max_address = 0xfe9f};
    constexpr MemoryObjectInfo g_memory_ppu_registers = {.min_address = 0xFF40, .max_address = 0xFF4B};

    struct Memory {
        RAM<g_memory_vram.size> vram{};
        RAM<g_memory_wram.size> wram{};
        RAM<g_memory_oam.size> oam{};
        RAM<g_memory_io_unused.size> unused_io{};
        RAM<g_memory_hram.size> hram{};
    };

    using VRAM = std::span<uint8_t, g_memory_vram.size>;
    using WRAM = std::span<uint8_t, g_memory_wram.size>;
    using OAM = std::span<uint8_t, g_memory_oam.size>;
    using HRAM = std::span<uint8_t, g_memory_hram.size>;
    using UnusedIO = std::span<uint8_t, g_memory_io_unused.size>;
} // namespace gb

#endif
