#pragma once
#include "gb/memory/Memory.h"

#include <cstdint>
#include <string>
#include <set>


namespace gb 
{
	class AddressBus 
	{
		using MemoryContainer = std::set<MemoryController, ControllerComparator>;
	public:
		AddressBus()
		{}

		~AddressBus() = default;

		inline void connect(const MemoryController& controller) { memory_.insert(controller); }
		inline void addObserver(const MemoryController& observer) { observers_.insert(observer); }
		inline void disconnect(const MemoryController& controller) { memory_.erase(controller); }

		size_t getObjectCount() const { return memory_.size(); }

		uint8_t read(uint16_t address) const;
		void write(uint16_t address, uint8_t data) const;
	public:
		class BusIterator
		{
			friend class AddressBus;
		public:
			BusIterator& operator++();
			BusIterator& operator--();
			BusIterator operator+(uint16_t value);
			BusIterator operator-(uint16_t value);
			BusIterator& operator+=(uint16_t value);
			BusIterator& operator-=(uint16_t value);
			BusIterator& operator=(const BusIterator& other);
			uint8_t operator*() const;
		private:
			BusIterator(MemoryContainer::iterator start, MemoryContainer::iterator end)
				: current_it_(start), end_it_(end)
			{}

			MemoryContainer::iterator current_it_;
			MemoryContainer::iterator end_it_;
			int current_address_;
		};
	private:
		std::string getErrorDescription(uint16_t address, int value = -1) const;

		MemoryContainer memory_;
		MemoryContainer observers_;
	};

}
