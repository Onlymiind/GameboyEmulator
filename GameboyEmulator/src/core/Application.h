#pragma once
#include "core/gb/RAM.h"
#include "core/gb/ROM.h"
#include "core/gb/AddressBus.h"
#include "core/gb/cpu/CPU.h"
#include "core/gb/IO.h"

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

		std::unique_ptr<RAM> m_RAM;
		std::unique_ptr<ROM> m_ROM;
		std::unique_ptr<RAM> m_Leftover;
		IORegisters m_GBIO;
		AddressBus m_Bus;
		std::unique_ptr<SharpSM83> m_CPU;

		const uint8_t m_InstructionPerFrame{ 100 };
		bool m_IsRunning;
		bool m_EmulatorRunning;
		bool m_StepMode;
		bool m_Execute;


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

		Command parse(const std::string& text) const;

	private:

		struct CommandInfo
		{
			std::string_view Name;
			CommandType Type;
			bool HasArguments;
		};

		std::string getArguments(const std::string& text, const CommandInfo& info) const;

		const std::array<CommandInfo, 6> m_Commands =
		{ {
			{"-help", CommandType::Help, false}, {"-quit", CommandType::Quit, false},
			{"-romdir", CommandType::SetRomDir, true}, {"-run", CommandType::RunRom, true},
			{"-ls", CommandType::List, false}, {"-config", CommandType::Config, false}
		} };
	};
}
