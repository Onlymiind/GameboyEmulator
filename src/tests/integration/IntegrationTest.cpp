#include "Application.h"
#include "gb/memory/Memory.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <string>
#include <sstream>
#include <memory>

std::string run_cmd = "-run ";

class TestOutputReader : public gb::MemoryObject
{
public:
	TestOutputReader(const emulator::Printer& printer)
		: printer_(printer)
	{}

	uint8_t read(uint16_t address) const override {
        return 0;
    }

	void write(uint16_t address, uint8_t data) override {
        //Used to get output from blargg's test ROMs.
        if(address == 0) {
            symbol = data;
        } else if (address == 0x01 && data == 0x81) {
	    	printer_.print(symbol);
	    }
    }
private:
	const emulator::Printer& printer_;
    uint8_t symbol = 0;
};

void runTestRom(const std::string& rom_name) {
    std::stringstream in(run_cmd + rom_name);
    std::stringstream dummy;
    emulator::Reader r(in);
    emulator::Printer p(dummy);
    std::unique_ptr<emulator::Application> app = std::make_unique<emulator::Application>(p, r, true);

    std::stringstream out;
    emulator::Printer printer(out);
    TestOutputReader test_out(printer);
    app->addMemoryObserver(0xFF01, 0xFF02, test_out);

    app->run();

    std::string d = dummy.str();
    std::string output = out.str();

    INFO(output);
    REQUIRE(output.find("Passed") != std::string::npos);
}

TEST_CASE("run cpu test roms") {
    auto name  = GENERATE(as<std::string>{}, 
        "01-special",
        "02-interrupts",
        "03-op sp,hl",
        "04-op r,imm",
        "05-op rp",
        "06-ld r,r",
        "07-jr,jp,call,ret,rst",
        "08-misc instrs",
        "09-op r,r",
        "10-bit ops",
        "11-op a,(hl)"
    );
    runTestRom(name);
}
