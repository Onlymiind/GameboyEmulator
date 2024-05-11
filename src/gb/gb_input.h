#ifndef GB_EMULATOR_SRC_GB_GB_INPUT_HDR_
#define GB_EMULATOR_SRC_GB_GB_INPUT_HDR_

#include "gb/interrupt_register.h"
#include "util/util.h"
#include <cstdint>

namespace gb {

    enum class Button {
        A = setBit(0),
        RIGHT = setBit(4),

        B = setBit(1),
        LEFT = setBit(5),

        SELECT = setBit(2),
        UP = setBit(6),

        START = setBit(3),
        DOWN = setBit(7)
    };

    constexpr uint8_t g_input_select_dpad = setBit(4);
    constexpr uint8_t g_input_select_buttons = setBit(5);

    struct InputState {
        bool a = false;
        bool b = false;
        bool select = false;
        bool start = false;
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
    };

    class Input {
      public:
        Input(InterruptRegister &interrupt_flags)
            : interrupt_flags_(interrupt_flags) {}

        uint8_t read() const {
            uint8_t value = 0xcf;
            if (select_dpad_) {
                value &= ~(state_ >> 4);
                value &= ~g_input_select_dpad;
            }
            if (select_buttons_) {
                value &= ~(state_ & 0xf);
                value &= ~g_input_select_buttons;
            }
            return value;
        }

        void write(uint8_t data) {
            select_buttons_ = (data & g_input_select_buttons) == 0;
            select_dpad_ = (data & g_input_select_dpad) == 0;
        }

        void setState(InputState state) {
            uint8_t old_state = state_;
            state_ = 0;
            state_ |= uint8_t(state.a);
            state_ |= uint8_t(state.b) << 1;
            state_ |= uint8_t(state.select) << 2;
            state_ |= uint8_t(state.start) << 3;
            state_ |= uint8_t(state.right) << 4;
            state_ |= uint8_t(state.left) << 5;
            state_ |= uint8_t(state.up) << 6;
            state_ |= uint8_t(state.down) << 7;
            if (select_buttons_ &&
                ((old_state & 0xf) | (state_ & 0xf)) != old_state) {
                interrupt_flags_.setFlag(InterruptFlags::JOYPAD);
            }
            if (select_buttons_ &&
                ((old_state >> 4) | (state_ >> 4)) != old_state) {
                interrupt_flags_.setFlag(InterruptFlags::JOYPAD);
            }
        }

      private:
        InterruptRegister &interrupt_flags_;
        uint8_t state_ = 0;
        bool select_dpad_ = false;
        bool select_buttons_ = false;
    };
} // namespace gb

#endif
