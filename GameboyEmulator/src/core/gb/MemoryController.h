#pragma once
#include <cstdint>
#include <functional>
#include <iostream>

namespace gb {


	class MemoryController {
	public:

		MemoryController(uint16_t minAddress, uint16_t maxAddress,
			std::function<uint8_t(uint16_t)> readCallback, std::function<void(uint16_t, uint8_t)> writeCallback) :
			m_MinAddress(minAddress), m_MaxAddress(maxAddress), m_ReadCallback(readCallback), m_WriteCallback(writeCallback) 
		{}
		MemoryController(const MemoryController& other) = default;

		~MemoryController() = default;

		inline uint8_t read(uint16_t address) { return m_ReadCallback(address - m_MinAddress); }
		inline void write(uint16_t address, uint8_t data) { m_WriteCallback(address - m_MinAddress, data); }

		inline bool isInRange(uint16_t address) const { return address >= m_MinAddress && address <= m_MaxAddress; }
	private:

		uint16_t m_MinAddress, m_MaxAddress;

		std::function<uint8_t(uint16_t)> m_ReadCallback;
		std::function<void(uint16_t, uint8_t)> m_WriteCallback;
	};
}