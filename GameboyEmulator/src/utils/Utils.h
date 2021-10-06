#pragma once

#include <type_traits>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <string_view>
#include <sstream>
#include <queue>
#include <functional>

std::vector<uint8_t> readFile(const std::filesystem::path& fileName);

template<typename T, typename std::enable_if<std::is_integral<T>{}, bool>::type = true >
inline void toHexOutput(std::stringstream& stream, T value)
{
	stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +value;
}

template<typename... Args>
std::string printToString(std::string_view separator, Args... args)
{
	std::stringstream stream;
	((stream << separator << args), ...);
	return stream.str();
}

template<typename Context, typename Result>
class Coroutine
{
public:
	template<typename Container>
	Coroutine(const Context& ctxt, const Result& res, const Container& instructions)
		: context_(ctxt), result_(res), instructions_(instructions)
	{}

	bool IsFinished() const;
	Result GetResult() const;

	void operator()();
private:
	Context context_;
	Result result_;
	std::queue<std::function<void(Context&, Result&)>> instructions_;
};

template<typename Context, typename Result>
bool Coroutine<Context, Result>::IsFinished() const
{
	return instructions_.empty();
}

template<typename Context, typename Result>
Result Coroutine<Context, Result>::GetResult() const
{
	return result_;
}

template<typename Context, typename Result>
void Coroutine<Context, Result>::operator()()
{
	if(instructions_.empty())
	{
		//TODO: maybe throw an exception
		return;
	}

	instructions_.front()(context_, result_);
	instructions_.pop();
}
