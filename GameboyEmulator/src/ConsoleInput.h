#pragma once
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
		CommandType type;
		std::string argument;
	};

	class Parser
	{
	public:
		Parser() = default;
		~Parser() = default;

		Command parse(std::string_view text) const;

	private:

		struct CommandInfo
		{
			CommandType type;
			bool has_arguments;
		};

		std::string getArguments(std::string_view text, const CommandInfo& info) const;

		const std::unordered_map<std::string, CommandInfo> commands_ =
		{
			{"-help", {CommandType::Help, false}}, {"-quit", {CommandType::Quit, false}},
			{"-romdir", {CommandType::SetRomDir, true}}, {"-run", {CommandType::RunRom, true}},
			{"-ls", {CommandType::List, false}}, {"-config", {CommandType::Config, false}}
		};
	};
}
