#include "Application.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"


int main() {
	emulator::Printer p(std::cout);
	emulator::Reader r(std::cin);

	emulator::Application* app = new emulator::Application(p, r);

	app->run();

	return 0;
}
