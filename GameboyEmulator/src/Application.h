#pragma once
#include "gb/RAM.h"
#include "gb/ROM.h"
#include "gb/AddressBus.h"
#include "gb/cpu/CPU.h"
#include "gb/IO.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Decoder.h"
#include "ConsoleInput.h"

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

		gb::RAM m_RAM;
		gb::ROM m_ROM;
		gb::RAM m_Leftover;
		gb::IORegisters m_GBIO;
		gb::AddressBus m_Bus;
		gb::InterruptRegister m_InterruptEnable;
		gb::InterruptRegister m_InterruptFlags;
		gb::decoding::Decoder m_Decoder;
		gb::cpu::SharpSM83 m_CPU;
		
		bool m_IsRunning;
		bool m_EmulatorRunning;
		bool m_JustStarted;


		std::string m_TestPath{ "../TestRoms/blargg/cpu_instrs/individual/" };
		const std::string m_Extension = ".gb";
		Parser m_CommandParser;
	};
}
