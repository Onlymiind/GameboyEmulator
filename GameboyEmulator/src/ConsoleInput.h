#pragma once
#include <iostream>
#include <string_view>
#include <string>
#include <unordered_map>

namespace emulator
{
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
			CommandType Type;
			bool HasArguments;
		};

		std::string getArguments(std::string_view text, const CommandInfo& info) const;

		const std::unordered_map<std::string, CommandInfo> m_Commands =
		{
			{"-help", {CommandType::Help, false}}, {"-quit", {CommandType::Quit, false}},
			{"-romdir", {CommandType::SetRomDir, true}}, {"-run", {CommandType::RunRom, true}},
			{"-ls", {CommandType::List, false}}, {"-config", {CommandType::Config, false}}
		};
	};
}
