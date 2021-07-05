#pragma once
#include "core/gb/MemoryObject.h"


#include <type_traits>
#include <cstdint>



namespace gb {


	class MemoryController {
	public:

		MemoryController(uint16_t minAddress, uint16_t maxAddress, MemoryObject& object)
			: m_MinAddress(minAddress), m_MaxAddress(maxAddress), m_MemoryObject(object) 
		{}
		MemoryController(const MemoryController& other)
			: m_MinAddress(other.m_MinAddress), m_MaxAddress(other.m_MaxAddress), m_MemoryObject(other.m_MemoryObject)
		{}

		~MemoryController() = default;

		inline uint8_t read(uint16_t address) const  { return m_MemoryObject.read(address - m_MinAddress); }
		inline void write(uint16_t address, uint8_t data) const  { m_MemoryObject.write(address - m_MinAddress, data); }

		inline bool isInRange(uint16_t address) const { return address >= m_MinAddress && address <= m_MaxAddress; }

		inline bool operator < (const MemoryController& other) const
		{
			return m_MaxAddress < other.m_MinAddress;
		}

		inline bool operator < (uint16_t address) const
		{
			return address > m_MaxAddress;
		}

		inline bool operator > (uint16_t address) const
		{
			return address < m_MinAddress;
		}
	private:

		uint16_t m_MinAddress, m_MaxAddress;
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
