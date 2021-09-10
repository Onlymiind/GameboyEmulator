#include "Application.h"
#include "ConsoleInput.h"
#include "ConsoleOutput.h"


int main() {
	emulator::Printer p(std::cout);

	emulator::Application* app = new emulator::Application(p);

	app->run();

	return 0;
}
