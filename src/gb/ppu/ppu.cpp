#include "gb/ppu/ppu.h"
#include "gb/interrupt_register.h"
#include "gb/memory/memory_map.h"
#include "util/util.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace gb {

    uint8_t PPU::readIO(uint16_t address) const {

        switch (IO(address)) {
        case IO::LCDC: return lcd_control_;
        case IO::LCD_STATUS:
            return status_ | (uint8_t(current_y_ == y_compare_) << 2) |
                   uint8_t(mode_);
        case IO::SCROLL_X: return scroll_x_;
        case IO::SCROLL_Y: return scroll_y_;
        case IO::LCD_Y: return current_y_;
        case IO::LYC: return y_compare_;
        case IO::DMA_SRC: return dma_src_;
        case IO::BG_PALETTE: return bg_palette_;
        case IO::OBJ0_PALETTE: return obj_palette0_;
        case IO::OBJ1_PALETTE: return obj_palette1_;
        case IO::WINDOW_Y: return window_y_;
        case IO::WINDOW_X: return window_x_;
        default: throw std::invalid_argument("unreachable"); return 0;
        }
    }

    void PPU::writeIO(uint16_t address, uint8_t data) {

        switch (IO(address)) {
        case IO::LCDC: lcd_control_ = data; break;
        case IO::LCD_STATUS: status_ = data & ~0b111; break;
        case IO::SCROLL_X: scroll_x_ = data; break;
        case IO::SCROLL_Y: scroll_y_ = data; break;
        case IO::LCD_Y: // read only
            break;
        case IO::LYC: y_compare_ = data; break;
        case IO::DMA_SRC: dma_src_ = data; break;
        case IO::BG_PALETTE: bg_palette_ = data; break;
        case IO::OBJ0_PALETTE: obj_palette0_ = data; break;
        case IO::OBJ1_PALETTE: obj_palette1_ = data; break;
        case IO::WINDOW_Y: window_y_ = data; break;
        case IO::WINDOW_X: window_x_ = data; break;
        default: throw std::invalid_argument("unreachable");
        }
    }

    uint8_t PPU::readVRAM(uint16_t address) const {
        if (!g_memory_vram.isInRange(address)) [[unlikely]] {
            throw std::invalid_argument("wrong VRAM address");
        }

        return vram_[address - g_memory_vram.min_address];
    }

    void PPU::writeVRAM(uint16_t address, uint8_t data) {
        if (!g_memory_vram.isInRange(address)) [[unlikely]] {
            throw std::invalid_argument("wrong VRAM address");
        }

        vram_[address - g_memory_vram.min_address] = data;
    }

    uint8_t PPU::readOAM(uint16_t address) const {
        if (!g_memory_oam.isInRange(address)) [[unlikely]] {
            throw std::invalid_argument("wrong VRAM address");
        }

        return oam_[address - g_memory_oam.min_address];
    }
    void PPU::writeOAM(uint16_t address, uint8_t data) {
        if (!g_memory_oam.isInRange(address)) [[unlikely]] {
            throw std::invalid_argument("wrong VRAM address");
        }

        oam_[address - g_memory_oam.min_address] = data;
    }

    void PPU::update() {
        if (!(lcd_control_ & LCDControlFlags::ENABLE)) {
            return;
        }

        switch (mode_) {
        case PPUMode::OAM_SCAN: {
            if (cycles_to_finish_ == g_oam_fetch_duration) {
                scanOAM();
            }
            break;
        }
        case PPUMode::RENDER:
            if (cycles_to_finish_ == g_render_duration) {
                renderPixelLine();
            }
            break;
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
                    interrupt_flags_.setFlag(InterruptFlags::VBLANK);
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
        set_interrupt = set_interrupt ||
                        (current_y_ == y_compare_ &&
                         (status_ & PPUInterruptSelectFlags::Y_COMPARE));
        if (set_interrupt) {
            interrupt_flags_.setFlag(InterruptFlags::LCD_STAT);
        }
    }

    std::array<GBColor, 8> PPU::getTileRow(uint16_t tilemap_base, uint8_t x,
                                           uint8_t y) {
        uint16_t tile_y = y / 8;
        uint16_t tile_x = x / 8;
        uint8_t tile_id = vram_[(tilemap_base + (tile_y * 32 + tile_x)) -
                                g_memory_vram.min_address];

        uint16_t tile_address = 0;
        if ((lcd_control_ & LCDControlFlags::BG_TILE_AREA) == 0) {
            tile_address = uint16_t(int(g_second_tile_data_block_offset) +
                                    int(int8_t(tile_id)) * 0x10);
        } else {
            tile_address = g_memory_vram.min_address + uint16_t(tile_id) * 0x10;
        }
        tile_address += (y % 8) * 2;

        return decodeTileRow(vram_[tile_address - g_memory_vram.min_address],
                             vram_[tile_address + 1 -
                                   g_memory_vram.min_address]);
    }

    GBColor PPU::getBGColor(GBColor color_idx) {
        return GBColor((bg_palette_ >> uint8_t(color_idx) * 2) & 0b11);
    }

    GBColor PPU::getSpriteColor(GBColor color_idx, bool use_obp1) {
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

    void PPU::renderPixelLine() {
        uint16_t tilemap_base = g_first_tilemap_offset;
        if (lcd_control_ & LCDControlFlags::BG_TILE_MAP) {
            tilemap_base = g_second_tilemap_offset;
        }
        std::vector<PixelInfo> pixels;
        pixels.resize(g_screen_width);

        std::array<GBColor, 8> current_tile{};

        // prefetch first tile if it is drawn partially
        if (scroll_x_ % 8 != 0) {
            current_tile = getTileRow(tilemap_base, scroll_x_,
                                      current_y_ + scroll_y_);
        }
        bool is_window = false;
        bool finished_sprites = false;
        for (uint8_t i = 0; i < g_screen_width; ++i) {
            if ((lcd_control_ & LCDControlFlags::BG_ENABLE)) {
                bool was_window = is_window;
                if (!is_window &&
                    (lcd_control_ & LCDControlFlags::WINDOW_ENABLE) != 0 &&
                    current_y_ >= window_y_ && (i + 7) >= window_x_) {
                    is_window = true;
                    if (lcd_control_ & LCDControlFlags::WINDOW_TILE_MAP) {
                        tilemap_base = g_second_tilemap_offset;
                    }
                }
                uint8_t x = 0;
                uint8_t y = 0;
                if (is_window) {
                    x = i - window_x_ + 7;
                    y = current_y_ - window_y_;
                } else {
                    x = i + scroll_x_;
                    y = current_y_ + scroll_y_;
                }
                uint8_t tile_offset = x % 8;
                if (tile_offset == 0 || (is_window && !was_window)) {
                    current_tile = getTileRow(tilemap_base, x, y);
                }
                pixels[i] = PixelInfo{.color_idx = current_tile[tile_offset],
                                      .palette = Palette::BG,
                                      .default_color = getBGColor(
                                          current_tile[tile_offset])};
            }
            if (!finished_sprites &&
                (lcd_control_ & LCDControlFlags::OBJ_ENABLE) &&
                !objects_on_current_line_.empty()) {
                auto it = std::lower_bound(objects_on_current_line_.begin(),
                                           objects_on_current_line_.end(), i,
                                           [](ObjectAttributes attrs,
                                              uint8_t x) {
                                               return attrs.x + 8 <= x;
                                           });
                if (it == objects_on_current_line_.end()) {
                    finished_sprites = true;
                    continue;
                } else if (it->x > i) {
                    continue;
                }
                GBColor pixel = getSpritePixel(*it, i);
                bool use_object_palette1 = it->use_object_palette1;
                while (pixel == GBColor::WHITE) {
                    ++it;
                    if (it == objects_on_current_line_.end()) {
                        break;
                    } else if (it->x > i) {
                        break;
                    }
                    pixel = getSpritePixel(*it, i);
                    use_object_palette1 = it->use_object_palette1;
                }
                if (pixel != GBColor::WHITE) {
                    pixels[i] = PixelInfo{.color_idx = pixel,
                                          .palette = Palette::OBP0,
                                          .default_color = getSpriteColor(
                                              pixel, use_object_palette1)};
                }
            }
        }

        if (renderer_) {
            renderer_->drawPixels(0, current_y_, pixels);
        }
    }

    void PPU::scanOAM() {
        objects_on_current_line_.clear();
        for (size_t i = 0; i < 40; ++i) {
            ObjectAttributes
                obj = decodeObjectAttributes(std::span<uint8_t, 4>{&oam_[i * 4],
                                                                   4},
                                             lcd_control_ &
                                                 LCDControlFlags::OBJ_SIZE);
            if (obj.y + 16 <= current_y_ ||
                (!(lcd_control_ & LCDControlFlags::OBJ_SIZE) &&
                 (obj.y + 8 <= current_y_))) {
                continue;
            }
            if (obj.y > current_y_) {
                continue;
            }

            objects_on_current_line_.push_back(obj);
        }

        // According to docs, if x coordinates of two OBJs are equal, the one
        // with lower OAM address is drawn. Thus, it is important to preserve
        // original memory order
        std::stable_sort(objects_on_current_line_.begin(),
                         objects_on_current_line_.end(),
                         [](auto lhs, auto rhs) { return lhs.x < rhs.x; });
    }

    GBColor PPU::getTilePixel(uint16_t row_start_address, uint8_t x_offset) {
        if (row_start_address + 1 >= g_memory_vram.size) {
            throw std::out_of_range("tile row address out of range");
        }
        if (x_offset > 7) {
            throw std::out_of_range("tile x offset out of range");
        }
        uint8_t lsb = vram_[row_start_address];
        uint8_t msb = vram_[row_start_address + 1];
        x_offset = 7 - x_offset;
        return static_cast<GBColor>(((lsb >> x_offset) & 1) |
                                    (((msb >> x_offset) & 1) << 1));
    }

    GBColor PPU::getSpritePixel(ObjectAttributes sprite, uint8_t current_x) {
        uint8_t size = lcd_control_ & LCDControlFlags::OBJ_SIZE ? 16 : 8;
        uint8_t row = sprite.flip_y ? size - (current_y_ - sprite.y) - 1
                                    : (current_y_ - sprite.y);
        uint16_t tile_idx = sprite.tile_idx;
        if (row >= 8) {
            row -= 8;
            ++tile_idx;
        }
        uint16_t tile_addr = tile_idx * 0x10 + row * 2;
        uint8_t x_offset = (current_x - sprite.x) & 7;
        if (sprite.flip_x) {
            x_offset = 7 - x_offset;
        }
        return getTilePixel(tile_addr, x_offset);
    }
} // namespace gb
