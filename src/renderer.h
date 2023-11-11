#pragma once

#include "gb/ppu/ppu.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace renderer {

    constexpr size_t g_bytes_per_pixel = 3;
    constexpr size_t g_red_offset = 0;
    constexpr size_t g_blue_offset = 1;
    constexpr size_t g_green_offset = 2;

    struct Color {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    constexpr Color from_hex(uint32_t val) {
        return Color{
            .red = uint8_t((val & 0xff0000) >> 16),
            .green = uint8_t((val & 0xff00) >> 8),
            .blue = uint8_t(val),
        };
    }

    constexpr std::array<Color, 4> g_default_palette{
        from_hex(0xe0f8d0),
        from_hex(0x88c070),
        from_hex(0x346856),
        from_hex(0x081820),
    };

    struct Palette {
        std::array<Color, 4> bg_palette{};
        std::array<Color, 4> obj_palette0{};
        std::array<Color, 4> obj_palette1{};
    };

    class Renderer : public gb::IRenderer {
      public:
        Renderer();

        void drawPixels(size_t x, size_t y, std::span<gb::PixelInfo> color) noexcept override;
        void finishFrame() noexcept override {}

        uint64_t getTextureID() const { return texture_id_; }

        void flush();

      private:
        void setPixel(size_t base_idx, Color color);

        std::optional<Palette> palette_;
        std::array<uint8_t, gb::g_screen_width * gb::g_screen_height * g_bytes_per_pixel> image_{};
        uint64_t texture_id_ = 0;
    };
} // namespace renderer
