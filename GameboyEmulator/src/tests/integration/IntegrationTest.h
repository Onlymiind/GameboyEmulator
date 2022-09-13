#pragma once
#include "gb/memory/Memory.h"
#include "ConsoleOutput.h"

#include <string>
#include <iostream>

class TestOutputReader : public gb::MemoryObject
{
public:
	TestOutputReader(const emulator::Printer& printer)
		: printer_(printer)
	{}

	uint8_t read(uint16_t address) const override;

	void write(uint16_t address, uint8_t data) override;
private:
	const emulator::Printer& printer_;
    uint8_t symbol = 0;
};

int runTestRom(const std::string& rom_name, std::ostream& out_stream);
