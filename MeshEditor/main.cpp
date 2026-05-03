#pragma once 

#include "Application.hpp"
//#include "Sphere.hpp"
//#include "Cube.hpp"
//#include "Split.hpp"

int main(int argc, char* argv[])
{
	Application app;

	//app.registerCommand(std::make_unique<Sphere>());
	//app.registerCommand(std::make_unique<Cube>());
	//app.registerCommand(std::make_unique<Split>());

	return app.execute(argc, argv);
}
