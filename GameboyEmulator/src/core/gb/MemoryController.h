#pragma once
#include <type_traits>
#include <cstdint>
#include <functional>



namespace gb {


	class MemoryController {
	public:

		MemoryController(uint16_t minAddress, uint16_t maxAddress,
			std::function<uint8_t(uint16_t)> readCallback, std::function<void(uint16_t, uint8_t)> writeCallback) :
			m_MinAddress(minAddress), m_MaxAddress(maxAddress), m_ReadCallback(readCallback), m_WriteCallback(writeCallback) 
		{}
		MemoryController(const MemoryController& other) = default;

		~MemoryController() = default;

		inline uint8_t read(uint16_t address) const  { return m_ReadCallback(address - m_MinAddress); }
		inline void write(uint16_t address, uint8_t data) const  { m_WriteCallback(address - m_MinAddress, data); }

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

		std::function<uint8_t(uint16_t)> m_ReadCallback;
		std::function<void(uint16_t, uint8_t)> m_WriteCallback;
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
