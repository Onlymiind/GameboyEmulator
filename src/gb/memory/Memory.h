#pragma once

#include <type_traits>
#include <cstdint>

namespace gb
{
    class MemoryObject
	{
	public:
		virtual uint8_t read(uint16_t address) const = 0;
		virtual void write(uint16_t address, uint8_t data) = 0;
	};

	class MemoryController
    {
	public:

		MemoryController(uint16_t minAddress, uint16_t maxAddress, MemoryObject& object)
			: min_address_(minAddress), max_address_(maxAddress), memory_object_(object) 
		{}
		MemoryController(const MemoryController& other)
			: min_address_(other.min_address_), max_address_(other.max_address_), memory_object_(other.memory_object_)
		{}

		~MemoryController() = default;

		uint8_t read(uint16_t address) const;
		void write(uint16_t address, uint8_t data) const;

		bool isInRange(uint16_t address) const;

		uint16_t GetMinAddress() const;
		uint16_t GetMaxAddress() const;

		bool operator < (const MemoryController& other) const;

		bool operator < (uint16_t address) const;

		bool operator > (uint16_t address) const;
	private:

		uint16_t min_address_, max_address_;
		MemoryObject& memory_object_;
	};

	struct ControllerComparator
	{
		using is_transparent = std::true_type;

		bool operator()(const MemoryController& lhs, const MemoryController& rhs) const;

		bool operator()(const MemoryController& lhs, uint16_t rhs) const;

		bool operator()(uint16_t lhs, const MemoryController& rhs) const;
	};
}