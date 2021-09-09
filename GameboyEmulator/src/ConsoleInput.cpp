#include "ConsoleInput.h"

#include <string>
#include <cstddef>

namespace emulator
{
    std::istream& operator>>(std::istream& is, Command& command)
    {
        std::string cmd;
        std::getline(is, cmd);

        command = Parser().parse(cmd);

        return is;
    }

    Command Parser::parse(std::string_view text) const
    {
        Command result = { CommandType::None, {} };

        size_t name_begin = text.find_first_not_of(' ');
        size_t name_end = text.find_first_of(' ', name_begin);
        if(name_end == std::string_view::npos)
        {
            name_end = text.size();
        }

        std::string cmd_name(text.data() + name_begin, name_end - name_begin);

        if(m_Commands.count(cmd_name))
        {
            result.Type = m_Commands.at(cmd_name).Type;
            result.Argument = getArguments(text.substr(name_end), m_Commands.at(cmd_name));

            if(m_Commands.at(cmd_name).HasArguments && result.Argument.empty())
            {
                result.Type = CommandType::Invalid;
            }
        }


        if (result.Type == CommandType::None && name_begin != std::string_view::npos)
        {
            result.Type = CommandType::Invalid;
        }
        

        return result;
    }

    std::string Parser::getArguments(std::string_view text, const Parser::CommandInfo& info) const
    {
        if (info.HasArguments)
        {
            size_t arg_pos = text.find_first_not_of(' ');
            size_t arg_end = text.find_last_not_of(' ');
            if (arg_pos != std::string::npos)
            {
                std::string_view arg = text.substr(arg_pos, arg_end - arg_pos + 1);
                return std::string(arg.data(), arg.size());
            }
            else
            {
                return {};
            }
        }

        return {};
    }
}
