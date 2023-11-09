#include "gb/ppu/ppu.h"
#include "gb/interrupt_register.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace gb {

    uint8_t PPU::read(uint16_t address) const {
        if (g_memory_vram.isInRange(address)) {
            // if (mode_ == PPUMode::RENDER) {
            //     return 0xff;
            // }
            return vram_[address - g_memory_vram.min_address];
        } else if (g_memory_oam.isInRange(address)) {
            // if (mode_ == PPUMode::OAM_SCAN || mode_ == PPUMode::RENDER) {
            //     return 0xff;
            // }
            return oam_[address - g_memory_oam.min_address];
        }

        switch (address) {
        case g_lcd_control_address: return lcd_control_;
        case g_lcd_status: return status_ | (uint8_t(current_y_ == y_compare_) << 2) | uint8_t(mode_);
        case g_scroll_x_address: return scroll_x_;
        case g_scroll_y_address: return scroll_y_;
        case g_lcd_y_address: return current_y_;
        case g_y_compare_address: return y_compare_;
        case g_dma_src_address: return dma_src_;
        case g_background_palette_address: return bg_palette_;
        case g_object_palette0_address: return obj_palette0_;
        case g_object_palette1_address: return obj_palette1_;
        case g_window_y_address: return window_y_;
        case g_window_x_address: return window_x_;
        }

        throw std::invalid_argument("unreachable");
        return 0;
    }

    void PPU::write(uint16_t address, uint8_t data) {
        if (g_memory_vram.isInRange(address)) {
            // if (mode_ == PPUMode::RENDER) {
            //     return;
            // }
            vram_[address - g_memory_vram.min_address] = data;
            return;
        } else if (g_memory_oam.isInRange(address)) {
            // if (mode_ == PPUMode::OAM_SCAN || mode_ == PPUMode::RENDER) {
            //     return;
            // }
            oam_[address - g_memory_oam.min_address] = data;
            return;
        }

        switch (address) {
        case g_lcd_control_address: lcd_control_ = data; break;
        case g_lcd_status: status_ = data & ~0b111; break;
        case g_scroll_x_address: scroll_x_ = data; break;
        case g_scroll_y_address: scroll_y_ = data; break;
        case g_lcd_y_address: // read only
            break;
        case g_y_compare_address: y_compare_ = data; break;
        case g_dma_src_address:
            dma_src_ = data;
            dma_running_ = true;
            // TODO: set DMA start address, implement DMA
            break;
        case g_background_palette_address: bg_palette_ = data; break;
        case g_object_palette0_address: obj_palette0_ = data; break;
        case g_object_palette1_address: obj_palette1_ = data; break;
        case g_window_y_address: window_y_ = data; break;
        case g_window_x_address: window_x_ = data; break;
        default: throw std::invalid_argument("unreachable");
        }
    }

    void PPU::update() {
        if (!(lcd_control_ & LCDControlFlags::ENABLE)) {
            return;
        }

        switch (mode_) {
        case PPUMode::OAM_SCAN: {
            // spend 80 clock cycles to check 40 y coordinates
            if (cycles_to_finish_ % 2 == 0) {
                // each OAM entry is 4 bytes long, y coordinate is the first byte in the entry
                size_t idx = (g_oam_fetch_duration - cycles_to_finish_) * 4;
                uint8_t y_coord = oam_[idx] - 16;
                // height = 8 if OBJ_SIZE bit is not set, 16 otherwise
                uint8_t height = 8 * (((lcd_control_ & LCDControlFlags::OBJ_SIZE) != 0) + 1);

                if (current_y_ >= y_coord && current_y_ < y_coord + height) {
                    ObjectAttributes attrs = decodeObjectAttributes(std::span<uint8_t, 4>{&oam_[idx], 4},
                                                                    (lcd_control_ & LCDControlFlags::OBJ_SIZE) != 0);
                    // discard objs that are not visible
                    if (attrs.x == 0) {
                        break;
                    }
                    auto it = std::upper_bound(objects_on_current_line_.begin(), objects_on_current_line_.end(), attrs,
                                               [](auto lhs, auto rhs) { return lhs.x < rhs.x; });

                    // if two objs have the same x coordinate, the obj with lower attributes address
                    // is drawn
                    if (it != objects_on_current_line_.begin() && (--it)->x == attrs.x) {
                        break;
                    }
                    objects_on_current_line_.insert(it, attrs);
                }
            }
            break;
        }
        case PPUMode::RENDER: renderPixelRow(); break;
        case PPUMode::HBLANK: break;
        case PPUMode::VBLANK:
            if (cycles_to_finish_ % g_scanline_duration == 1) {
                ++current_y_;
            }
            break;
        default: throw std::runtime_error("unexpected PPU mode");
        }

        --cycles_to_finish_;
        bool set_interrupt = false;
        bool finish_frame = false;
        if (cycles_to_finish_ == 0) {
            switch (mode_) {
            case PPUMode::OAM_SCAN:
                mode_ = PPUMode::RENDER;
                cycles_to_finish_ = g_render_duration;

                // init obj queue
                objects_to_draw_ = objects_on_current_line_;
                break;
            case PPUMode::RENDER:
                mode_ = PPUMode::HBLANK;
                cycles_to_finish_ = g_scanline_duration - g_render_duration;
                set_interrupt = status_ & PPUInterruptSelectFlags::HBLANK;

                // reset obj queue
                objects_on_current_line_.clear();
                objects_to_draw_ = std::span<ObjectAttributes>{};
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
                current_x_ = 0;
                break;
            case PPUMode::VBLANK:
                mode_ = PPUMode::OAM_SCAN;
                cycles_to_finish_ = g_oam_fetch_duration;
                current_y_ = 0;
                frame_finished_ = true;
                if (renderer_) {
                    renderer_->finishFrame();
                }
                break;
            default: throw std::runtime_error("unexpected PPU mode");
            }
        }
        set_interrupt = set_interrupt || (current_y_ == y_compare_ && (status_ & PPUInterruptSelectFlags::Y_COMPARE));
        if (set_interrupt) {
            interrupt_flags_.setFlag(InterruptFlags::LCD_STAT);
        }
    }

    void PPU::renderPixelRow() {
        if (current_x_ >= g_screen_width) {
            return;
        }
        std::vector<PixelInfo> pixels;
        pixels.reserve(8);

        if (lcd_control_ & LCDControlFlags::BG_ENABLE) {
            uint16_t tilemap_base = g_first_tilemap_offset;
            if (lcd_control_ & LCDControlFlags::BG_TILE_MAP) {
                tilemap_base = g_second_tilemap_offset;
            }
            auto row = getTileRow(tilemap_base, current_x_ + scroll_x_, current_y_ + scroll_y_);

            // used for scrolling individual pixels in background rendering
            uint8_t pixel_offset = 0;
            if (cycles_to_finish_ == g_render_duration) { // first BG tile drawn this scanline
                pixel_offset = scroll_x_ % 8;
            }
            for (size_t i = pixel_offset; i < 8; ++i) {
                pixels.push_back(PixelInfo{
                    .color_idx = row[i],
                    .default_color = getBGColor(row[i]),
                });
            }

            if ((lcd_control_ & LCDControlFlags::WINDOW_ENABLE) != 0 && current_y_ >= window_y_ &&
                (current_x_ + 7) >= window_x_) {
                if (lcd_control_ & LCDControlFlags::WINDOW_TILE_MAP) {
                    tilemap_base = g_second_tilemap_offset;
                }

                auto window_row = getTileRow(tilemap_base, current_x_ - window_x_ + 7, current_y_ - window_y_);
                uint8_t tile_column = (current_x_ - window_x_) % 8;
                for (uint8_t i = tile_column; i < std::min(size_t(8), pixels.size()); ++i) {
                    pixels[i] = PixelInfo{
                        .color_idx = window_row[i - tile_column],
                        .default_color = getBGColor(window_row[i - tile_column]),
                    };
                }
            }
        }

        if ((lcd_control_ & LCDControlFlags::OBJ_ENABLE) && !objects_to_draw_.empty()) {
            size_t discarded_start = 0;
            for (size_t i = 0; i < objects_to_draw_.size() && objects_to_draw_[i].x <= current_x_ + 8; ++i) {

                ObjectAttributes obj = objects_to_draw_[i];

                uint8_t size = lcd_control_ & LCDControlFlags::OBJ_SIZE ? 16 : 8;
                uint8_t row = obj.flip_y ? size - (current_y_ + 16 - obj.y) : (current_y_ + 16 - obj.y);
                // address can overflow into the next tile since sprites can be two tiles tall
                // current_y_ >= obj.y
                uint16_t tile_addr = uint16_t(obj.tile_idx) * 0x10 + row * 2;
                std::array<GBColor, 8> obj_pixels = decodeTileRow(vram_[tile_addr], vram_[tile_addr + 1]);

                if (obj.flip_x) {
                    // reverse the array
                    std::swap(obj_pixels[0], obj_pixels[7]);
                    std::swap(obj_pixels[1], obj_pixels[6]);
                    std::swap(obj_pixels[2], obj_pixels[5]);
                    std::swap(obj_pixels[3], obj_pixels[4]);
                }
                size_t pixels_start = std::max(obj.x, uint8_t(current_x_ + 8)) - obj.x;
                size_t count = std::min(size_t(8 - pixels_start), pixels.size());
                for (size_t i = 0; i < count; ++i) {
                    if (obj_pixels[pixels_start + i] == GBColor::WHITE) {
                        // transparent pixel
                        continue;
                    } else if ((pixels[i].palette == Palette::OBP0 || pixels[i].palette == Palette::OBP1) &&
                               pixels[i].color_idx != GBColor::WHITE) {
                        // do not overwrite already drawn objs
                        continue;
                    } else if (pixels[i].palette == Palette::BG && obj.priority &&
                               pixels[i].color_idx != GBColor::WHITE) {
                        // handle ObjectAttributes.priority flag
                        continue;
                    }

                    pixels[i] = PixelInfo{
                        .color_idx = obj_pixels[pixels_start + i],
                        .palette = obj.use_object_palette1 ? Palette::OBP1 : Palette::OBP0,
                        .default_color = getSpriteColor(obj_pixels[pixels_start + i], obj.use_object_palette1),
                    };
                }

                if (obj.x + 8 <= current_x_ + 8 + pixels.size()) {
                    // obj will not be drawn later
                    ++discarded_start;
                }
            }
            objects_to_draw_ = objects_to_draw_.subspan(discarded_start);
        }

        if (pixels.empty()) {
            pixels.insert(pixels.end(), 8, PixelInfo{});
        }
        if (renderer_) {
            renderer_->drawPixels(current_x_, current_y_, pixels);
        }
        current_x_ += pixels.size();
    }

    std::array<GBColor, 8> PPU::getTileRow(uint16_t tilemap_base, uint8_t x, uint8_t y) {
        uint16_t tile_y = y / 8;
        uint16_t tile_x = x / 8;
        uint8_t tile_id = vram_[(tilemap_base + (tile_y * 32 + tile_x)) - g_memory_vram.min_address];

        uint16_t tile_address = 0;
        if ((lcd_control_ & LCDControlFlags::BG_TILE_AREA) == 0) {
            tile_address = uint16_t(int(g_second_tile_data_block_offset) + int(int8_t(tile_id)) * 0x10);
        } else {
            tile_address = g_memory_vram.min_address + uint16_t(tile_id) * 0x10;
        }
        tile_address += (y % 8) * 2;

        return decodeTileRow(vram_[tile_address - g_memory_vram.min_address],
                             vram_[tile_address + 1 - g_memory_vram.min_address]);
    }

    GBColor PPU ::getBGColor(GBColor color_idx) { return GBColor((bg_palette_ >> uint8_t(color_idx) * 2) & 0b11); }

    GBColor PPU ::getSpriteColor(GBColor color_idx, bool use_obp1) {
        if (use_obp1) {
            return GBColor((obj_palette1_ >> uint8_t(color_idx) * 2) & 0b11);
        }
        return GBColor((obj_palette0_ >> uint8_t(color_idx) * 2) & 0b11);
    }

    void PPU::reset() {
        memset(vram_.data(), 0, vram_.size());
        current_y_ = 0;
        y_compare_ = 0;
        window_x_ = 0;
        window_y_ = 0;
        scroll_x_ = 0;
        scroll_y_ = 0;
        mode_ = PPUMode::VBLANK;
        cycles_to_finish_ = 1;
        bg_palette_ = 0xfc;
        lcd_control_ = 0x91;
    }
} // namespace gb
