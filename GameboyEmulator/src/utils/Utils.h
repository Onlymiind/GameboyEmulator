#pragma once
#include "SFML/Graphics/Color.hpp"
#include "glm/vec4.hpp"

#include <type_traits>
#include <sstream>
#include <string>

inline sf::Color colorToSFML(glm::vec4 color)
{	
	glm::u8vec4 unnormalized(color * 255.0f);

	return { unnormalized.r, unnormalized.g, unnormalized.b, unnormalized.a };
}


template<typename T, typename std::enable_if < std::is_integral<T>{}, bool >::type = true >
inline std::string toHexOutput(T value)
{
	std::stringstream stream{};
	stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +value;
	return stream.str();
}

template<typename T, typename std::enable_if < std::is_integral<T>{}, bool > ::type = true >
inline void toHexOutput(std::stringstream& stream, T value)
{
	stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +value;
}