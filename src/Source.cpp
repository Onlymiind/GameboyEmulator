#include "Application.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"


int main() {
	emulator::Printer p(std::cout);
	emulator::Reader r(std::cin);

	std::unique_ptr<emulator::Application> app = std::make_unique<emulator::Application>(p, r);

	app->run();

	return 0;
}
