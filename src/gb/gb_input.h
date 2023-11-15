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
    };

    class Input {
      public:
        Input(InterruptRegister &interrupt_flags) : interrupt_flags_(interrupt_flags) {}

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
            select_buttons_ = (data & g_input_select_buttons) != 0;
            select_dpad_ = (data & g_input_select_dpad) != 0;
        }

        void setState(uint8_t state) { state_ = state; }

      private:
        InterruptRegister &interrupt_flags_;
        uint8_t state_ = 0;
        bool select_dpad_ = false;
        bool select_buttons_ = false;
    };
} // namespace gb

#endif
