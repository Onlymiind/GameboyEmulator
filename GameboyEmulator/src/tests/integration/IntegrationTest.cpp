#include "Application.h"
#include "utils/MemoryObserver.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"

#include <filesystem>
#include <exception>
#include <string>
#include <sstream>
#include <iostream>

std::string input = 
R"(-run 01-special
-run 02-interrupts
-run 03-op sp,hl
-run 04-op r,imm
-run 05-op rp
-run 06-ld r,r
-run 07-jr,jp,call,ret,rst
-run 08-misc instrs
-run 09-op r,r
-run 10-bit ops
-run 11-op a,(hl)
-quit)";

class TestOutputReader : public emulator::MemoryObserver
{
public:
	TestOutputReader(const emulator::Printer& printer)
		: printer_(printer)
	{}

	uint8_t read(uint16_t address) override
	{
		return memory_->read(address);
	}

	void write(uint16_t address, uint8_t data) override
	{
		if (address == 0x02 && data == 0x81)
		{
			printer_.print(memory_->read(0x01));
		}

		memory_->write(address, data);
	}
private:
	const emulator::Printer& printer_;
};

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cout << "Expected single argument: path to project directory" << std::endl;
        return 1;
    }
    std::filesystem::path proj_path(argv[1]);
    std::filesystem::path rom_path = proj_path / "src" / "tests" / "integration" / "roms";

    if(!std::filesystem::exists(rom_path))
    {
        std::cout << "Project path is invalid or directory <path>/src/tests/integration/roms/ doesn't exist\n" << std::endl;
        return 1;
    }
    std::filesystem::current_path(proj_path / "src" / "tests" / "integration" / "roms");
    
    std::stringstream in(input);
    emulator::Reader r(in);
    emulator::Printer p(std::cout);
    emulator::Application* app = new emulator::Application(p, r);

    TestOutputReader tests_out(p);

    app->addMemoryObserver(emulator::MemoryType::IO, tests_out);

    app->run();

    std::cin.get();

    return 0;
}
