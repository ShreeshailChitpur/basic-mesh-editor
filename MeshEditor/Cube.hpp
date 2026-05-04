#pragma once

#include "Command.hpp"
#include "Geometry.hpp"

/**
 * @class Cube
 * @brief Represents a command to create a cube.
 *
 * The Cube class defines a command that creates a cube of specified dimensions
 * and exports it in STL format using a TriangleSoup structure.
 */
class Cube : public Command
{
public:
	/**
	 * @brief Gets the name of the command.
	 * @return A constant reference to the command name as a std::string.
	 *
	 * This function overrides Command::getName and returns the unique identifier
	 * of the Cube command.
	 */
	const std::string& getName() const override;

	/**
	 * @brief Executes the cube creation command.
	 * 
	 * Expected arguments:
	 * - `"L"`: Length of the cube's sides.
	 * - `"origin"`: Comma-separated origin vector.
	 * - `"filepath"`: Path to output STL file.
	 * 
	 * Error return codes:
	 * - `0` : Success.
	 * - `1` : Invalid length (zero or negative).
	 * - `2` : File I/O error (unable to read or write).
	 * - `3` : Invalid or missing arguments.
	 *
	 * @param args A map containing command arguments such as length and output path.
	 * @return 0 on success, non-zero on failure.
	 *
	 * This function parses the arguments, creates a cube, and saves it to a file.
	 */
	int execute(const std::map<std::string, std::string>& args) override;

private:
	double length = 0.0;                 /**< Length of the cube's sides. */
	Vec origin;							/**< Origin of the cube in 3D space. */
	std::string filepath;               /**< Path to save the generated STL file. */

	/**
	 * @brief Generates a triangle mesh representing the cube.
	 *
	 * This method constructs and returns a complete triangular mesh of the cube
	 * based on the current origin and length parameters. The resulting mesh is
	 * returned as a TriangleSoup object.
	 *
	 * @return A TriangleSoup containing all triangles that make up the cube.
	 */
	TriangleSoup createCube() const;
};
