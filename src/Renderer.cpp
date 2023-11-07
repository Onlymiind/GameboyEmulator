#include "Renderer.h"
#include "gb/ppu/PPU.h"

#include "glad/gl.h"
#include <cstdint>

#include <GL/gl.h>
#include <functional>

namespace renderer {
    Renderer::Renderer() {
        GLuint id{};
        glGenTextures(1, &id);
        texture_id_ = uint64_t(id);
        GLint last_active{};
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_active);
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gb::g_screen_width, gb::g_screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     image_.data());
        glBindTexture(GL_TEXTURE_2D, last_active);
    }

    void Renderer::drawPixels(size_t x, size_t y, std::span<gb::PixelInfo> pixels) noexcept {
        if (y >= gb::g_screen_height || x >= gb::g_screen_width) {
            return;
        }

        size_t pixel_idx = 0;
        for (size_t i = (y * gb::g_screen_width + x) * 3; pixel_idx < pixels.size() && i + 2 < image_.size();
             i += 3, ++pixel_idx) {
            switch (pixels[i].palette) {
            case gb::Palette::BG:
                setPixel(i, palette_ ? palette_->bg_palette[size_t(pixels[i].color_idx)]
                                     : g_default_palette[size_t(pixels[i].default_color)]);
                break;
            case gb::Palette::OBP0:
                setPixel(i, palette_ ? palette_->obj_palette0[size_t(pixels[i].color_idx)]
                                     : g_default_palette[size_t(pixels[i].default_color)]);
                break;
            case gb::Palette::OBP1:
                setPixel(i, palette_ ? palette_->obj_palette1[size_t(pixels[i].color_idx)]
                                     : g_default_palette[size_t(pixels[i].default_color)]);
                break;
            }
        }
    }

    void Renderer::setPixel(size_t base_idx, Color color) {
        image_[base_idx] = color.red;
        image_[base_idx + 1] = color.green;
        image_[base_idx + 2] = color.blue;
    }

    void Renderer::finishFrame() noexcept {
        GLint last_active;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_active);
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gb::g_screen_width, gb::g_screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     image_.data());
        glBindTexture(GL_TEXTURE_2D, last_active);
    }
} // namespace renderer
