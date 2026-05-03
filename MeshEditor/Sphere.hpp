#pragma once

#include "Command.hpp"
#include "Geometry.hpp"

/**
 * @class Sphere
 * @brief Represents a command to create a 3D sphere geometry and export it to an STL file.
 *
 * The Sphere class defines a command that generates a triangulated 3D mesh
 * representing a sphere of a specified radius, origin, and subdivision level
 * (controlled by the number of slices and rings). The generated mesh can be
 * written to an STL file via the STLParser utility.
 *
 * Command arguments:
 * - R: Radius of the sphere (must be > 0)
 * - origin: Comma-separated coordinates of the sphere's center (e.g. "0,0,0")
 * - slices: Number of vertical subdivisions around the sphere (longitude lines)
 * - rings: Number of horizontal subdivisions from pole to pole (latitude lines)
 * - filepath: Output file path for the generated STL
 *
 * Error codes returned by execute():
 * - 0: Success
 * - 1: Invalid radius (zero or negative)
 * - 2: Invalid slices/rings configuration
 * - 3: Incorrect filepath
 * - 4: Missing or invalid arguments
 */
class Sphere : public Command
{
public:
    /**
     * @brief Retrieves the name of the command.
     *
     * This function overrides the Command::getName() method
     * and returns the identifier string for this command ("Sphere").
     *
     * @return A constant reference to the command name.
     */
    const std::string& getName() const override;

    /**
     * @brief Executes the sphere creation command.
     *
     * Parses the provided arguments to extract the sphere parameters,
     * constructs a triangulated sphere mesh, and writes it to an STL file.
     *
     * @param args A map of argument names to values:
     *   - "R" (radius)
     *   - "origin" (center position)
     *   - "slices" (number of vertical subdivisions)
     *   - "rings" (number of horizontal subdivisions)
     *   - "filepath" (output file path)
     *
     * @return 0 on success, or a nonzero error code (see class description).
     */
    int execute(const std::map<std::string, std::string>& args) override;

private:
    /**
     * @brief Generates a triangulated mesh representation of the sphere.
     *
     * Creates a collection of triangles (TriangleSoup) that approximate the sphere's surface,
     * based on the configured radius, origin, slices, and rings.
     *
     * @return A TriangleSoup object containing the generated triangles.
     */
    TriangleSoup createSphere() const;

    /**
     * @brief Converts spherical coordinates to Cartesian coordinates.
     *
     * Converts a point defined by spherical coordinates (theta, phi) into
     * a 3D position (x, y, z) in Cartesian space, relative to the current sphere origin.
     *
     * @param theta Polar angle (0 at the north pole to PI at the south pole).
     * @param phi Azimuthal angle around the equator (0 to 2*PI).
     * @return A Vec representing the computed Cartesian coordinates.
     */
    Vec sphericalToCartesian(double theta, double phi) const;

private:
    double radius = 0.0;       /**< Radius of the sphere. */
    Vec origin;                /**< Origin (center) of the sphere in 3D space. */
    int nSlices = 0;           /**< Number of subdivisions around the sphere (longitude). */
    int nRings = 0;            /**< Number of subdivisions from pole to pole (latitude). */
    std::string filepath;      /**< Path to save the generated STL file. */
};
