#ifndef GB_EMULATOR_SRC_GB_MEMORY_MEMORY_MAP_HDR_
#define GB_EMULATOR_SRC_GB_MEMORY_MEMORY_MAP_HDR_

#include <cstdint>

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
}

#endif
