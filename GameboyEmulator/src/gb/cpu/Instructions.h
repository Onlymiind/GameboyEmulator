#pragma once
#include "gb/cpu/CPUUtils.h"
#include "gb/cpu/Operation.h"

#include <memory>

namespace gb
{
    namespace cpu
    {
        //TODO: implement instructions as different pipelines
        //This shouldn't be difficult and seems like good idea
        class Pipeline
        {
        public:
            virtual ~Pipeline() = default;

            virtual bool nextStage() = 0;

            static std::unique_ptr<Pipeline> newPipeline(const decoding::Decoder& decoder, RegisterFile& registers, decoding::opcode opcode);
        protected:
            RegisterFile& registers_;
            const decoding::Decoder& decoder_;
            decoding::opcode opcode_;
            uint8_t stage_count_ = 0;
        };

        class ALUPipeline : public Pipeline
        {
        public:
        private:
        };

        class ADDpipeline : public Pipeline
        {

        };

        class LDPipeline : public Pipeline
        {

        };

        class JRPipeline : public Pipeline
        {

        };

        class JPPipeline : public Pipeline
        {

        };

        class CALLpipeline : public Pipeline
        {

        };

        void addByte(InstructionContext& context);
        void addSP(InstructionContext& context);
        void addHL(InstructionContext& context);
        void adc(InstructionContext& context);
        void sub(InstructionContext& context);
        void sbc(InstructionContext& context);
        void decByte(InstructionContext& context);
        void decWord(InstructionContext& context);
        void incByte(InstructionContext& context);
        void incWord(InstructionContext& context);
        void and(InstructionContext& context);
        void xor(InstructionContext& context);
        void or(InstructionContext& context);
        void cp(InstructionContext& context);
        void daa(InstructionContext& context);
        void cpl(InstructionContext& context);
        void ccf(InstructionContext& context);
        void scf(InstructionContext& context);
    }
}