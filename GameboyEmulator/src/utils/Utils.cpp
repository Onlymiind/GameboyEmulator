#include "Utils.h"

#include <cstdint>

size_t computeSizeFromAddresses(uint16_t first, uint16_t last)
{
	return static_cast<size_t>(last) - first + 1;
}
