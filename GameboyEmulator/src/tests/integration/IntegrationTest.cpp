#include "Application.h"
#include "gb/memory/Memory.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"

#include <filesystem>
#include <exception>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

std::string run_cmd = "-run ";
std::string separator = "------------------------------";

std::vector<std::string> tests = {
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
};

class TestOutputReader : public gb::MemoryObject
{
public:
	TestOutputReader(const emulator::Printer& printer)
		: printer_(printer)
	{}

	uint8_t read(uint16_t address) const override
	{
		return 0;
	}

	void write(uint16_t address, uint8_t data) override
	{
        //Used to get output from blargg's test ROMs.
        if(address == 0)
        {
            symbol = data;
        }
		else if (address == 0x01 && data == 0x81)
		{
			printer_.print(symbol);
		}
	}
private:
	const emulator::Printer& printer_;
    uint8_t symbol = 0;
};

void runTestRom(const std::string& rom_name) {
    std::stringstream in(run_cmd + rom_name);
    std::stringstream out;
    std::stringstream dummy;
    emulator::Reader r(in);
    emulator::Printer p(dummy);
    std::unique_ptr<emulator::Application> app = std::make_unique<emulator::Application>(p, r, true);

    TestOutputReader test_out(out);
    app->addMemoryObserver(0xFF01, 0xFF02, test_out);

    app->run();

    std::string d = dummy.str();
    std::string output = out.str();
    if(output.find("Passed") != std::string::npos) {
        std::cout << "Test passed: " << rom_name << '\n';
    } else {
        output = output.substr(output.find(rom_name) + rom_name.length());
        output = output.substr(output.find_first_not_of('\n'));
        std::cout << "Test failed: " << rom_name << '\n' << output << '\n';
    }

    std::cout << separator << '\n';
}

int main() {
    namespace  fs = std::filesystem;
    fs::path rom_path = fs::current_path() / "src" / "tests" / "integration" / "roms";

    if(!fs::exists(rom_path)) {
        std::cout << "Project path is invalid or directory" << rom_path << " doesn't exist\n" << std::endl;
        return 1;
    }
    fs::current_path(rom_path);
    
    for(const auto& rom : tests) {
        runTestRom(rom);
    }

    return 0;
}
