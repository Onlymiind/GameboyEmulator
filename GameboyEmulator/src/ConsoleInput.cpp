#include "ConsoleInput.h"

#include <string>
#include <cstddef>

namespace emulator
{

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

        if(commands_.count(cmd_name))
        {
            result.type = commands_.at(cmd_name).type;
            result.argument = getArguments(text.substr(name_end), commands_.at(cmd_name));

            if(commands_.at(cmd_name).has_arguments && result.argument.empty())
            {
                result.type = CommandType::Invalid;
            }
        }


        if (result.type == CommandType::None && name_begin != std::string_view::npos)
        {
            result.type = CommandType::Invalid;
        }
        

        return result;
    }

    std::string Parser::getArguments(std::string_view text, const Parser::CommandInfo& info) const
    {
        if (info.has_arguments)
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
