#ifndef GB_EMULATOR_SRC_GB_PPU_PPU_HDR_
#define GB_EMULATOR_SRC_GB_PPU_PPU_HDR_

#include "gb/interrupt_register.h"
#include "gb/memory/basic_components.h"
#include "util/util.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace gb {

    constexpr MemoryObjectInfo g_memory_vram = {.min_address = 0x8000, .max_address = 0x9fff};
    constexpr MemoryObjectInfo g_memory_oam = {.min_address = 0xe000, .max_address = 0xfe9f};
    constexpr MemoryObjectInfo g_memory_ppu_registers = {.min_address = 0xFF40, .max_address = 0xFF4B};
    constexpr uint16_t g_lcd_control_address = 0xFF40;
    constexpr uint16_t g_lcd_status = 0xFF41;
    constexpr uint16_t g_scroll_y_address = 0xFF42;
    constexpr uint16_t g_scroll_x_address = 0xFF43;
    constexpr uint16_t g_lcd_y_address = 0xFF44;
    constexpr uint16_t g_y_compare_address = 0xFF45;
    constexpr uint16_t g_dma_src_address = 0xFF46;
    constexpr uint16_t g_background_palette_address = 0xFF47;
    constexpr uint16_t g_object_palette0_address = 0xFF48;
    constexpr uint16_t g_object_palette1_address = 0xFF49;
    constexpr uint16_t g_window_y_address = 0xFF4A;
    constexpr uint16_t g_window_x_address = 0xFF4B;

    constexpr uint16_t g_second_tile_data_block_offset = 0x9000;
    constexpr uint16_t g_first_tilemap_offset = 0x9800;
    constexpr uint16_t g_second_tilemap_offset = 0x9C00;

    constexpr uint16_t g_tilemap_dimension = 256;

    constexpr size_t g_oam_fetch_duration = 80;
    constexpr size_t g_render_duration = 172;
    constexpr size_t g_rener_extra_duration = 12;
    constexpr size_t g_scanline_duration = 456;
    constexpr size_t g_vblank_scanlines = 10;
    constexpr size_t g_screen_width = 160;
    constexpr size_t g_screen_height = 143;
    constexpr size_t g_vblank_duration = g_scanline_duration * g_vblank_scanlines;

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

    constexpr inline uint8_t operator|(LCDControlFlags lhs, LCDControlFlags rhs) { return uint8_t(lhs) | uint8_t(rhs); }
    constexpr inline uint8_t operator|(uint8_t lhs, LCDControlFlags rhs) { return lhs | uint8_t(rhs); }

    enum class PPUInterruptSelectFlags : uint8_t {
        HBLANK = setBit(3),
        VBLANK = setBit(4),
        OAM_SCAN = setBit(5),
        Y_COMPARE = setBit(6)
    };

    enum class ObjectAttribbutesFlags : uint8_t {
        USE_OBJECT_PALETTE1 = setBit(4),
        FLIP_X = setBit(5),
        FLIP_Y = setBit(6),
        PRIORITY = setBit(7)
    };

    constexpr inline uint8_t operator&(uint8_t value, LCDControlFlags flags) { return value & uint8_t(flags); }

    constexpr inline uint8_t operator&(uint8_t value, PPUInterruptSelectFlags flags) { return value & uint8_t(flags); }

    constexpr inline uint8_t operator&(uint8_t value, ObjectAttribbutesFlags flags) { return value & uint8_t(flags); }

    struct ObjectAttributes {
        // y posiytion + 16
        uint8_t y = 0;
        // x position + 8
        uint8_t x = 0;
        // NOTE: for 8*16 tiles this is an index of the first tile
        // and is rounded down to the even number
        uint8_t tile_idx = 0;

        bool priority = false;
        bool flip_y = false;
        bool flip_x = false;
        bool use_object_palette1 = false;
    };

    enum class GBColor : uint8_t { WHITE, LIGHT_GRAY, DARK_GRAY, BLACK };
    enum class Palette : uint8_t { BG, OBP0, OBP1 };

    struct PixelInfo {
        GBColor color_idx = GBColor::WHITE;
        Palette palette = Palette::BG;
        GBColor default_color = GBColor::WHITE;
    };

    class IRenderer {
      public:
        virtual void drawPixels(size_t x, size_t y, std::span<PixelInfo> color) noexcept = 0;
        virtual void finishFrame() noexcept = 0;

      protected:
        ~IRenderer() = default;
    };

    struct PixelFIFO {};

    class PPU {
      public:
        PPU(InterruptRegister &interrupt_flags) : interrupt_flags_(interrupt_flags) {
            objects_on_current_line_.reserve(40);
            reset();
        }
        PPU(InterruptRegister &interrupt_flags, IRenderer &renderer)
            : interrupt_flags_(interrupt_flags), renderer_(&renderer) {
            objects_on_current_line_.reserve(40);
            reset();
        }

        uint8_t readIO(uint16_t address) const;
        void writeIO(uint16_t address, uint8_t data);

        uint8_t readVRAM(uint16_t address) const;
        void writeVRAM(uint16_t address, uint8_t data);

        uint8_t readOAM(uint16_t address) const;
        void writeOAM(uint16_t address, uint8_t data);

        void update();

        void setRenderer(IRenderer &renderer) { renderer_ = &renderer; }
        void removeRenderer() { renderer_ = nullptr; }
        void renderPixelRow();

        PPUMode getMode() const { return mode_; }

        bool frameFinished() const { return frame_finished_; }
        void resetFrameFinistedFlag() { frame_finished_ = false; }

        void reset();

      private:
        std::array<GBColor, 8> getTileRow(uint16_t tilemap_base, uint8_t x, uint8_t y);
        GBColor getBGColor(GBColor color_idx);
        GBColor getSpriteColor(GBColor color_idx, bool use_obp1);

        std::vector<ObjectAttributes> objects_on_current_line_;
        std::span<ObjectAttributes> objects_to_draw_;
        size_t cycles_to_finish_ = g_vblank_duration;
        InterruptRegister &interrupt_flags_;
        IRenderer *renderer_ = nullptr;
        uint8_t current_x_ = 0;

        PPUMode mode_ = PPUMode::VBLANK;
        bool dma_running_ = false;
        bool frame_finished_ = false;
        uint16_t current_dma_address_ = 0;

        // memory-mapped registers
        // FIXME: LCD probably should be turned off at startup
        uint8_t lcd_control_ = LCDControlFlags::ENABLE | LCDControlFlags::BG_ENABLE | LCDControlFlags::WINDOW_ENABLE |
                               LCDControlFlags::OBJ_ENABLE;
        uint8_t status_ = 0;
        uint8_t scroll_x_ = 0;
        uint8_t scroll_y_ = 0;
        uint8_t current_y_ = g_screen_height + 1;
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

    constexpr inline std::array<GBColor, 8> decodeTileRow(uint8_t low, uint8_t high) {
        std::array<GBColor, 8> result{};
        // bit 7 is the leftmost pixel of the line
        for (size_t i = 0; i < 8; ++i, low >>= 1, high >>= 1) {
            result[7 - i] = GBColor((low & 1) | ((high & 1) << 1));
        }
        return result;
    }

    constexpr inline ObjectAttributes decodeObjectAttributes(std::span<uint8_t, 4> raw, bool double_height) {
        ObjectAttributes result{.y = raw[1], .x = raw[0], .tile_idx = raw[2]};
        if (double_height) {
            result.tile_idx &= 0xFE;
        }

        result.priority = (raw[3] & ObjectAttribbutesFlags::PRIORITY) != 0;
        result.flip_y = (raw[3] & ObjectAttribbutesFlags::FLIP_Y) != 0;
        result.flip_x = (raw[3] & ObjectAttribbutesFlags::FLIP_X) != 0;
        result.use_object_palette1 = (raw[3] & ObjectAttribbutesFlags::USE_OBJECT_PALETTE1) != 0;

        return result;
    }
} // namespace gb

#endif
