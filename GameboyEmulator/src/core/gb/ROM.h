#pragma once
#include <vector>
#include <cstdint>
#include <utility>

namespace gb {



	class ROM {
	public:
		ROM(std::vector<uint8_t>&& data) :
			m_Data(std::move(data)) 
		{}
		~ROM() = default;

		inline uint8_t read(uint16_t address) { return m_Data[address]; }
		inline void write(uint16_t address, uint8_t data) {/*Do nothing*/}
	private:
		std::vector<uint8_t> m_Data;
	};
}