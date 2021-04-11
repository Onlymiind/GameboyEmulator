#pragma once
#include "SFML/Graphics/Color.hpp"
#include "glm/vec4.hpp"

inline sf::Color colorToSFML(glm::vec4 color)
{	
	glm::u8vec4 unnormalized(color * 255.0f);

	return { unnormalized.r, unnormalized.g, unnormalized.b, unnormalized.a };
}