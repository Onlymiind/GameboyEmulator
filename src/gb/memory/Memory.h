#pragma once

#include <type_traits>
#include <cstdint>

namespace gb {

    class MemoryObject {
	public:
		virtual uint8_t read(uint16_t address) const = 0;
		virtual void write(uint16_t address, uint8_t data) = 0;
	};

	class MemoryController {
	public:

		MemoryController(uint16_t minAddress, uint16_t maxAddress, MemoryObject& object)
			: min_address_(minAddress), max_address_(maxAddress), memory_object_(&object) 
		{}
		MemoryController(const MemoryController& other)
			: min_address_(other.min_address_), max_address_(other.max_address_), memory_object_(other.memory_object_)
		{}

		~MemoryController() = default;

		uint8_t read(uint16_t address) const { return memory_object_->read(address - min_address_); }
		void write(uint16_t address, uint8_t data) const { memory_object_->write(address - min_address_, data); }

		bool isInRange(uint16_t address) const { return address >= min_address_ && address <= max_address_; }

		uint16_t getMinAddress() const { return min_address_; }
		uint16_t getMaxAddress() const { return max_address_; }

		bool operator < (const MemoryController& other) const { return max_address_ < other.min_address_; }

		bool operator < (uint16_t address) const { return address > max_address_; }

		bool operator > (uint16_t address) const { return address < min_address_; }
	private:

		uint16_t min_address_, max_address_;
		MemoryObject* memory_object_;
	};

	struct ControllerComparator {
		using is_transparent = std::true_type;

		bool operator()(const MemoryController& lhs, const MemoryController& rhs) const { return lhs < rhs; }

		bool operator()(const MemoryController& lhs, uint16_t rhs) const { return lhs < rhs; }

		bool operator()(uint16_t lhs, const MemoryController& rhs) const { return rhs > lhs; }
	};
}
