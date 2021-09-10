#pragma once
#include "gb/RAM.h"
#include "gb/ROM.h"
#include "gb/AddressBus.h"
#include "gb/cpu/CPU.h"
#include "gb/IO.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"

#include <memory>
#include <string_view>


namespace emulator
{
	class Application 
	{
	public:
		Application();
		~Application();


		void run();

		void draw();

	private:

		void init();
		void update();
		void pollCommands();
		void cleanup();

	private:

		gb::RAM RAM_;
		gb::ROM ROM_;
		gb::RAM leftover_;
		gb::IORegisters GBIO_;
		gb::AddressBus bus_;
		gb::InterruptRegister interrupt_enable_;
		gb::InterruptRegister interrupt_flags_;
		gb::decoding::Decoder decoder_;
		gb::cpu::SharpSM83 CPU_;
		
		bool is_running_;
		bool emulator_running_;
		bool just_started_;


		std::string test_path_ = "../TestRoms/blargg/cpu_instrs/individual/";
		const std::string extension_ = ".gb";
		Parser command_parser_;
		Printer printer_;
	};
}
