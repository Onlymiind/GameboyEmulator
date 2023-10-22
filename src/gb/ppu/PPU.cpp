#include "gb/ppu/PPU.h"
#include <stdexcept>

namespace gb {

    uint8_t PPU::read(uint16_t address) const {
        if (g_memory_vram.isInRange(address)) {
            return vram_[address - g_memory_vram.min_address];
        } else if (g_memory_oam.isInRange(address)) {
            return oam_[address - g_memory_oam.min_address];
        }

        switch (address) {
        case g_lcd_control_address:
            return lcd_control_;
        case g_lcd_status:
            return status_ | (uint8_t(current_y_ == y_compare_) << 2) | uint8_t(mode_);
        case g_scroll_x_address:
            return scroll_x_;
        case g_scroll_y_address:
            return scroll_y_;
        case g_lcd_y_address:
            return current_y_;
        case g_y_compare_address:
            return y_compare_;
        case g_dma_src_address:
            return dma_src_;
        case g_background_palette_address:
            return bg_palette_;
        case g_object_palette0_address:
            return obj_palette0_;
        case g_object_palette1_address:
            return obj_palette1_;
        case g_window_y_address:
            return window_y_;
        case g_window_x_address:
            return window_x_;
        }

        throw std::invalid_argument("unreachable");
        return 0;
    }

    void PPU::write(uint16_t address, uint8_t data) {
        if (g_memory_vram.isInRange(address)) {
            vram_[address - g_memory_vram.min_address] = data;
        } else if (g_memory_oam.isInRange(address)) {
            oam_[address - g_memory_oam.min_address] = data;
        }

        switch (address) {
        case g_lcd_control_address:
            lcd_control_ = data;
            break;
        case g_lcd_status:
            status_ = data & ~0b111;
            break;
        case g_scroll_x_address:
            scroll_x_ = data;
            break;
        case g_scroll_y_address:
            scroll_y_ = data;
            break;
        case g_lcd_y_address:
            break;
        case g_y_compare_address:
            y_compare_ = data;
        case g_dma_src_address:
            dma_src_ = data;
            dma_running_ = true;
            // TODO: set DMA start address, implement DMA
            break;
        case g_background_palette_address:
            bg_palette_ = data;
            break;
        case g_object_palette0_address:
            obj_palette0_ = data;
            break;
        case g_object_palette1_address:
            obj_palette1_ = data;
            break;
        case g_window_y_address:
            window_y_ = data;
            break;
        case g_window_x_address:
            window_x_ = data;
            break;
        default:
            throw std::invalid_argument("unreachable");
        }
    }
} // namespace gb
