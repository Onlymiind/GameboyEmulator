#pragma once
#include "gb/MemoryController.h"

#include <cstdint>
#include <string>
#include <set>


namespace gb {


	class AddressBus {
	public:
		AddressBus()
		{}

		~AddressBus() = default;

		inline void connect(const MemoryController& controller) { memory_.insert(controller); }

		size_t getObjectCount() const { return memory_.size(); }

		uint8_t read(uint16_t address) const;
		void write(uint16_t address, uint8_t data) const;
	private:
		std::string getErrorDescription(uint16_t address, int value = -1) const;

		std::set<MemoryController, ControllerComparator> memory_;
	};

}
