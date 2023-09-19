#pragma once
#include <iostream>
#include <string_view>
#include <string>
#include <unordered_map>

namespace emulator
{
    enum class CommandType
	{
		None, Invalid, Help, Quit, SetRomDir, RunRom, List, Config, LaunchGUI
	};

	struct Command
	{
		CommandType type;
		std::string argument;
	};

	class Reader
	{
	public:
		Reader(std::istream& input)
			:input_(input)
		{}
		~Reader() = default;

		std::string getLine() const;
		Command parse(std::string_view text) const;

		template<typename T>
		void get(T& value) const;

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
			{"-ls", {CommandType::List, false}}, {"-config", {CommandType::Config, false}},
			{"-gui",{CommandType::LaunchGUI, false}}
		};

		std::istream& input_;
	};

	template<typename T>
	void Reader::get(T& value) const
	{
		input_ >> value;
	}
}
