#pragma once

#include <type_traits>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <filesystem>
#include <string_view>
#include <sstream>
#include <deque>
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

template<typename Context>
class Coroutine
{
public:
	template<typename Container>
	Coroutine(const Context& ctxt, const Container& instructions)
		: context_(ctxt), instructions_(instructions.begin(), instructions.end())
	{}

	bool IsFinished() const;

	//For testing
	const Context& GetContext() const;

	void operator()();
private:
	Context context_;
	std::deque<std::function<void(Context&)>> instructions_;
};

template<typename Context>
bool Coroutine<Context>::IsFinished() const
{
	return instructions_.empty();
}

template<typename Context>
const Context& Coroutine<Context>::GetContext() const
{
	return context_;
}

template<typename Context>
void Coroutine<Context>::operator()()
{
	if(instructions_.empty())
	{
		//TODO: maybe throw an exception
		return;
	}

	instructions_.front()(context_);
	instructions_.pop_front();
}
