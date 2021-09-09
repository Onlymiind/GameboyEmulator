#pragma once
#include "gb/RAM.h"
#include "gb/ROM.h"
#include "gb/AddressBus.h"
#include "gb/cpu/CPU.h"
#include "gb/IO.h"
#include "gb/InterruptRegister.h"
#include "gb/cpu/Decoder.h"

#include <memory>
#include <string_view>


namespace gb {

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

		RAM m_RAM;
		ROM m_ROM;
		RAM m_Leftover;
		IORegisters m_GBIO;
		AddressBus m_Bus;
		InterruptRegister m_InterruptEnable;
		InterruptRegister m_InterruptFlags;
		decoding::Decoder m_Decoder;
		SharpSM83 m_CPU;
		

		const uint8_t m_InstructionPerFrame{ 100 };
		bool m_IsRunning;
		bool m_EmulatorRunning;
		bool m_JustStarted;


		std::string m_TestPath{ "../TestRoms/blargg/cpu_instrs/individual/" };
		const std::string m_Extension = ".gb";
	};

	enum class CommandType
	{
		None, Invalid, Help, Quit, SetRomDir, RunRom, List, Config
	};

	struct Command
	{
		CommandType Type;
		std::string Argument;
	};

	std::istream& operator >> (std::istream& is, Command& cmd);

	class Parser
	{
	public:
		Parser() = default;
		~Parser() = default;

		Command parse(std::string_view text) const;

	private:

		struct CommandInfo
		{
			std::string_view Name;
			CommandType Type;
			bool HasArguments;
		};

		std::string getArguments(std::string_view text, const CommandInfo& info) const;

		const std::array<CommandInfo, 6> m_Commands =
		{ {
			{"-help", CommandType::Help, false}, {"-quit", CommandType::Quit, false},
			{"-romdir", CommandType::SetRomDir, true}, {"-run", CommandType::RunRom, true},
			{"-ls", CommandType::List, false}, {"-config", CommandType::Config, false}
		} };
	};
}
