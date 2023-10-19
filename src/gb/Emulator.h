#pragma once
#include "gb/cpu/CPUUtils.h"
#include "gb/memory/BasicComponents.h"
#include "gb/AddressBus.h"
#include "gb/cpu/CPU.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Decoder.h"
#include "gb/Timer.h"
#include "utils/Utils.h"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace gb {

    class Emulator {
    public:
        Emulator() = default;

        Emulator(bool break_on_infinite_loop)
            : Emulator()
        { break_on_infinite_loop_ = break_on_infinite_loop; }

        cpu::RegisterFile getRegisters() const { return cpu_.getRegisters(); }

        uint16_t getPC() const { return cpu_.getProgramCounter(); }

        void setMemoryObserver(MemoryObserver& observer) { bus_.setObserver(observer); }
        void removeMemoryObserver() { bus_.removeObserver(); }

        void tick();

        bool terminated() const { return !is_running_; }

        void reset() { cpu_.reset(); }

        void setROM(std::vector<uint8_t> rom) { bus_.setRomData(std::move(rom)); }

        void start() { is_running_ = true; }

        void stop() { is_running_ = false; }

        bool instructionFinished() const { return cpu_.isFinished(); }

        cpu::Instruction getLastInstruction() const { return cpu_.getLastInstruction(); }

        uint8_t peekMemory(uint16_t address) { return bus_.read(address); }

    private:
        AddressBus bus_;
        cpu::SharpSM83 cpu_{bus_};

        bool is_running_ = false;
        bool break_on_infinite_loop_ = true;
    };

    inline void Emulator::tick() {
        if(!is_running_) {
            return;
        }
        static uint16_t oldPC = cpu_.getProgramCounter();

        try {
            cpu_.tick();
            for(int i = 0; i < 4; ++i) {
                bus_.update();
            }
        }
        catch(...) {
            is_running_ = false;
            throw;
        }

        if(cpu_.isFinished()) {
            if(break_on_infinite_loop_ && oldPC == cpu_.getProgramCounter() && !cpu_.isHalted()) {
                is_running_ = false;
                std::stringstream s;
                s << "reached infinite loop at address 0x" << std::setfill('0') << std::setw(sizeof(oldPC) * 2) << std::hex << oldPC;
                throw std::runtime_error(s.str());
            }

            oldPC = cpu_.getProgramCounter();
        }
    }
}
