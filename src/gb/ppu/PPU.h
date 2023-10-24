#pragma once
#include "gb/memory/BasicComponents.h"
#include "utils/Utils.h"
#include <cstdint>

namespace gb {

    constexpr MemoryObjectInfo g_memory_vram = {.min_address = 0x8000, .max_address = 0x9fff};
    constexpr MemoryObjectInfo g_memory_oam = {.min_address = 0xe000, .max_address = 0xfe9f};
    constexpr uint16_t g_lcd_control_address = 0xFF40;
    constexpr uint16_t g_lcd_status = 0xFF41;
    constexpr uint16_t g_scroll_x_address = 0xFF42;
    constexpr uint16_t g_scroll_y_address = 0xFF43;
    constexpr uint16_t g_lcd_y_address = 0xFF44;
    constexpr uint16_t g_y_compare_address = 0xFF45;
    constexpr uint16_t g_dma_src_address = 0xFF46;
    constexpr uint16_t g_background_palette_address = 0xFF47;
    constexpr uint16_t g_object_palette0_address = 0xFF48;
    constexpr uint16_t g_object_palette1_address = 0xFF49;
    constexpr uint16_t g_window_y_address = 0xFF4A;
    constexpr uint16_t g_window_x_address = 0xFF4B;

    enum class PPUMode : uint8_t { HBLANK = 0, VBLANK = 1, OAM_SCAN = 2, RENDER = 3 };

    enum class LCDControlFlags : uint8_t {
        BG_ENABLE = setBit(0),
        OBJ_ENABLE = setBit(1),
        OBJ_SIZE = setBit(2),
        BG_TILE_MAP = setBit(3),
        BG_TILE_AREA = setBit(4),
        WINDOW_ENABLE = setBit(5),
        WINDOW_TILE_MAP = setBit(6),
        ENABLE = setBit(7)
    };

    enum class PPUInterruptSelectFlags : uint8_t {
        HBLANK = setBit(3),
        VBLANK = setBit(4),
        OAM_SCAN = setBit(5),
        Y_COMPARE = setBit(6)
    };

    class PPU {
      public:
        uint8_t read(uint16_t address) const;
        void write(uint16_t address, uint8_t data);

      private:
        PPUMode mode_ = PPUMode::VBLANK;
        bool dma_running_ = false;
        uint16_t current_dma_address_ = 0;

        // memory-mapped registers
        uint8_t lcd_control_ = 0;
        uint8_t status_ = 0;
        uint8_t scroll_x_ = 0;
        uint8_t scroll_y_ = 0;
        uint8_t current_y_ = 0;
        uint8_t y_compare_ = 0;
        uint8_t dma_src_ = 0;
        uint8_t bg_palette_ = 0;
        uint8_t obj_palette0_ = 0;
        uint8_t obj_palette1_ = 0;
        uint8_t window_x_ = 0;
        uint8_t window_y_ = 0;

        RAM<g_memory_vram.size> vram_{};
        RAM<g_memory_oam.size> oam_{};
    };
} // namespace gb
