#pragma once
#include "gb/MemoryObject.h"


#include <vector>
#include <cstdint>
#include <utility>

namespace gb {



	class ROM:public MemoryObject {
	public:
		ROM(std::vector<uint8_t>&& data) :
			m_Data(std::move(data)) 
		{}

		ROM() = default;

		~ROM() = default;

		inline void setData(std::vector<uint8_t>&& data) { m_Data = std::move(data); }
		inline uint8_t read(uint16_t address) override { return m_Data[address]; }
		inline void write(uint16_t address, uint8_t data) override {/*Do nothing*/}
	private:
		std::vector<uint8_t> m_Data;
	};
}
