#pragma once
#include "gb/MemoryObject.h"


#include <type_traits>
#include <cstdint>



namespace gb {


	class MemoryController {
	public:

		MemoryController(uint16_t minAddress, uint16_t maxAddress, MemoryObject& object)
			: min_address_(minAddress), max_address_(maxAddress), m_MemoryObject(object) 
		{}
		MemoryController(const MemoryController& other)
			: min_address_(other.min_address_), max_address_(other.max_address_), m_MemoryObject(other.m_MemoryObject)
		{}

		~MemoryController() = default;

		inline uint8_t read(uint16_t address) const  { return m_MemoryObject.read(address - min_address_); }
		inline void write(uint16_t address, uint8_t data) const  { m_MemoryObject.write(address - min_address_, data); }

		inline bool isInRange(uint16_t address) const { return address >= min_address_ && address <= max_address_; }

		inline bool operator < (const MemoryController& other) const
		{
			return max_address_ < other.min_address_;
		}

		inline bool operator < (uint16_t address) const
		{
			return address > max_address_;
		}

		inline bool operator > (uint16_t address) const
		{
			return address < min_address_;
		}
	private:

		uint16_t min_address_, max_address_;
		MemoryObject& m_MemoryObject;
	};

	struct ControllerComparator
	{
		using is_transparent = std::true_type;

		bool operator()(const MemoryController& lhs, const MemoryController& rhs) const
		{
			return lhs < rhs;
		}

		bool operator()(const MemoryController& lhs, uint16_t rhs) const
		{
			return lhs < rhs;
		}

		bool operator()(uint16_t lhs, const MemoryController& rhs) const
		{
			return rhs > lhs;
		}
	};
}
