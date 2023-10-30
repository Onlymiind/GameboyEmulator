#pragma once

#include "gb/ppu/PPU.h"
#include <array>
#include <cstddef>
#include <cstdint>
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

    constexpr std::array<Color, 4> g_default_palette{};

    struct Palette {
        std::array<Color, 4> bg_palette{};
        std::array<Color, 4> obj_palette0{};
        std::array<Color, 4> obj_palette1{};
    };

    class Renderer : public gb::IRenderer {
      public:
        Renderer();

        void drawPixels(size_t x, size_t y, std::span<gb::PixelInfo> color) noexcept override;
        void finishFrame() noexcept override;

      private:
        void setPixel(size_t base_idx, Color color);

        std::optional<Palette> palette_;
        std::array<uint8_t, gb::g_screen_width * gb::g_screen_height * g_bytes_per_pixel> image_;
        uint64_t texture_id_ = 0;
    };
} // namespace renderer
