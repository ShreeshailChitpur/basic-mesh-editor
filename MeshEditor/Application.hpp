#pragma once

#include "Command.hpp"

#include <memory>
#include <map>
#include <string>

/**
 * @brief Main application class that manages and executes commands.
 *
 * The Application class is responsible for registering commands
 * and dispatching them based on command-line arguments.
 */
class Application
{
public:
    /**
     * @brief Registers a command with the application.
     *
     * Adds a new command to the internal command map. The command
     * is associated with its name, as returned by Command::getName().
     *
     * @param command A unique pointer to the Command to register.
     */
	void registerCommand(std::unique_ptr<Command> command);

    /**
     * @brief Executes the appropriate command based on command-line arguments.
     *
     * Parses the arguments and looks up the corresponding command
     * to execute. Additional arguments after the command name are passed
     * to the command itself.
     *
     * @param argc Argument count.
     * @param argv Argument vector.
     * @return The exit code returned by the executed command.
     */
	int execute(int argc, char* argv[]);

private:
    /**
     * @brief Map of command names to their corresponding command instances.
     */
	std::map<std::string, std::unique_ptr<Command>> commands;
};
