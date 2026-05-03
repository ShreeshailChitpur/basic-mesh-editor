#include "Application.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <span>

void Application::registerCommand(std::unique_ptr<Command> command)
{
	commands.emplace(command->getName(), std::move(command));
}

int Application::execute(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Error: No command provided.";
		return 3;
	}

	std::string commandName = argv[1];
	auto it = commands.find(commandName);

	if (it == commands.end())
	{
		std::cerr << "Error: Unknown command.";
		return 3;
	}

	std::map<std::string, std::string> args;

	for (std::string_view arg : std::span(argv + 2, argc - 2))
	{
		size_t eqPos = arg.find('=');
		if (eqPos != std::string::npos)
		{
			args.emplace(
				std::string{ arg.substr(0, eqPos) },
				std::string{ arg.substr(eqPos + 1) }
			);
		}
	}

	return it->second->execute(args);
}
