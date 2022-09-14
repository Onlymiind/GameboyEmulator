#include "Application.h"
#include "gb/memory/Memory.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"
#include "tests/integration/IntegrationTest.h"

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

std::string run_cmd = "-run ";
std::string separator = "------------------------------";

uint8_t TestOutputReader::read(uint16_t address) const {
    return 0;
}

void TestOutputReader::write(uint16_t address, uint8_t data) {
    //Used to get output from blargg's test ROMs.
    if(address == 0) {
        symbol = data;
    }
	else if (address == 0x01 && data == 0x81) {
		printer_.print(symbol);
	}
}

int runTestRom(const std::string& rom_name, std::ostream& out_stream) {
    std::stringstream in(run_cmd + rom_name);
    std::stringstream out;
    std::stringstream dummy;
    emulator::Reader r(in);
    emulator::Printer p(dummy);
    emulator::Printer printer(out);
    std::unique_ptr<emulator::Application> app = std::make_unique<emulator::Application>(p, r, true);

    TestOutputReader test_out(printer);
    app->addMemoryObserver(0xFF01, 0xFF02, test_out);

    app->run();

    std::string d = dummy.str();
    std::string output = out.str();
    if(output.find("Passed") != std::string::npos) {
        out_stream << "Test passed: " << rom_name << '\n';
    } else {
        output = output.substr(output.find(rom_name) + rom_name.length());
        output = output.substr(output.find_first_not_of('\n'));
        out_stream << "Test failed: " << rom_name << '\n' << output << '\n';
        return 1;
    }
    return 0;
}
