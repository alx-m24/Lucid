#include <iostream>
#include "Lucid/gui.hpp"

int main() {
	Lucid::Init("demo");

	std::string test = "";
	Lucid::BindInput("test", &test);

	while (!Lucid::WindowShouldClose()) {
		Lucid::Update();
	}

	Lucid::Terminate();

	return EXIT_SUCCESS;
}