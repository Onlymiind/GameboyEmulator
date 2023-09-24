#include "Application.h"


int main() {

	std::unique_ptr<emulator::Application> app = std::make_unique<emulator::Application>();

	app->run();

	return 0;
}
