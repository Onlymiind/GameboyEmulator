#include "gb/ppu/PPU.h"
#include "gb/InterruptRegister.h"
#include <cstdint>
#include <stdexcept>

namespace gb {

    uint8_t PPU::read(uint16_t address) const {
        if (g_memory_vram.isInRange(address)) {
            if (mode_ == PPUMode::RENDER) {
                return 0xff;
            }
            return vram_[address - g_memory_vram.min_address];
        } else if (g_memory_oam.isInRange(address)) {
            if (mode_ == PPUMode::OAM_SCAN || mode_ == PPUMode::RENDER) {
                return 0xff;
            }
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
            if (mode_ == PPUMode::RENDER) {
                return;
            }
            vram_[address - g_memory_vram.min_address] = data;
        } else if (g_memory_oam.isInRange(address)) {
            if (mode_ == PPUMode::OAM_SCAN || mode_ == PPUMode::RENDER) {
                return;
            }
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
        case g_lcd_y_address: // read only
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

    void PPU::tick() {
        if (!(lcd_control_ & LCDControlFlags::ENABLE)) {
            return;
        } else if (mode_ == PPUMode::RENDER && cycles_to_finish_ <= g_rener_extra_duration) {
            return;
        }

        switch (mode_) {
        case PPUMode::OAM_SCAN: {
            // spend 80 clock cycles to check 40 y coordinates
            if (cycles_to_finish_ % 2 == 0) {
                // each OAM entry is 4 bytes long, y coordinate is the first byte in the entry
                size_t idx = (g_oam_fetch_duration - cycles_to_finish_) * 4;
                uint8_t y_coord = oam_[idx];
                // height = 8 if OBJ_SIZE bit is not set, 16 otherwise
                uint8_t height = 8 * (((lcd_control_ & LCDControlFlags::OBJ_SIZE) != 0) + 1);

                if (current_y_ >= y_coord && current_y_ <= y_coord + height) {
                    objects_on_current_line_.push_back(
                        decodeObjectAttributes(std::span<uint8_t, 4>{&oam_[idx], 4},
                                               (lcd_control_ & LCDControlFlags::OBJ_SIZE) != 0));
                }
            }
            break;
        }
        case PPUMode::RENDER:
            renderPixelRow();
            break;
        case PPUMode::HBLANK:
            break;
        case PPUMode::VBLANK:
            if (cycles_to_finish_ % g_scanline_duration == g_scanline_duration - 1) {
                ++current_y_;
            }
            break;
        default:
            throw std::runtime_error("unexpected PPU mode");
        }

        --cycles_to_finish_;
        bool set_interrupt = false;
        bool finish_frame = false;
        if (cycles_to_finish_ == 0) {
            switch (mode_) {
            case PPUMode::OAM_SCAN:
                mode_ = PPUMode::RENDER;
                cycles_to_finish_ = g_render_duration;
                break;
            case PPUMode::RENDER:
                mode_ = PPUMode::HBLANK;
                cycles_to_finish_ = g_scanline_duration - g_render_duration;
                set_interrupt = status_ & PPUInterruptSelectFlags::HBLANK;
                break;
            case PPUMode::HBLANK:
                if (current_y_ == g_screen_height) {
                    mode_ = PPUMode::VBLANK;
                    cycles_to_finish_ = g_vblank_duration;
                    set_interrupt = status_ & PPUInterruptSelectFlags::VBLANK;
                } else {
                    mode_ = PPUMode::OAM_SCAN;
                    cycles_to_finish_ = g_oam_fetch_duration;
                    set_interrupt = status_ & PPUInterruptSelectFlags::OAM_SCAN;
                }
                ++current_y_;
                break;
            case PPUMode::VBLANK:
                mode_ = PPUMode::OAM_SCAN;
                current_y_ = 0;
                if (renderer_) {
                    renderer_->finishFrame();
                }
                break;
            default:
                throw std::runtime_error("unexpected PPU mode");
            }
        }
        set_interrupt = set_interrupt || (current_y_ == y_compare_ &&
                                          (status_ & PPUInterruptSelectFlags::Y_COMPARE));
        if (set_interrupt) {
            interrupt_flags_.setFlag(InterruptFlags::LCD_STAT);
        }
    }
    void PPU::renderPixelRow() {}
} // namespace gb
