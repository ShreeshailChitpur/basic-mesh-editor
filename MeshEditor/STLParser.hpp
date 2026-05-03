#pragma once

#include "Geometry.hpp"

#include <vector>
#include <string>
#include <sstream>

/**
 * @class STLParser
 * @brief Parses and writes 3D mesh data in ASCII STL format.
 *
 * The STLParser class provides functionality to:
 * - Read triangle data from ASCII STL files.
 * - Write triangle data to ASCII STL files.
 * - Compute normals for triangles when not provided in the file.
 */
class STLParser 
{
public:
    /**
     * @brief Parses an ASCII STL file and extracts its triangle mesh data.
     *
     * This function reads a file line by line, identifies vertex definitions, groups
     * them into triangles, calculates the normal vector for each triangle, and returns
     * the complete mesh.
     *
     * @param filename Path to the input ASCII STL file.
     * @return TriangleSoup A collection of parsed triangles. Returns empty if the file can't be opened.
     */
    TriangleSoup read(const std::string& filename);

    /**
     * @brief Writes triangle mesh data to an ASCII STL file.
     *
     * This function outputs the triangle data in valid STL syntax. It computes the
     * normal vector for each triangle before writing to ensure correct formatting.
     *
     * @param triangleSoup The triangle mesh to write.
     * @param filename Path to the output ASCII STL file.
     */
    void write(const TriangleSoup& triangleSoup, const std::string& filename);

private:
    /**
     * @brief Parses a single vertex line from an STL file.
     *
     * The line is expected to be in the format: "vertex x y z".
     *
     * @param line The line from the STL file containing vertex data.
     * @return Vec A Vec structure representing the vertex coordinates.
     */
    Vec parseVertexLine(const std::string_view& line);

    /**
     * @brief Parses a normal vector line from an STL file.
     *
     * The line is expected to be in the format: "facet normal nx ny nz".
     *
     * @param line The line from the STL file containing the normal vector.
     * @return Vec A Vec structure representing the normal vector.
     */
    Vec parseNormalLine(const std::string_view& line);

    /**
     * @brief Reads a single facet (triangle) from the STL file stream.
     *
     * This function assumes the file stream is positioned at a facet normal line.
     * It reads the normal vector and the three vertex lines of the triangle.
     * If the normal vector is missing or zero, it calculates it from the vertices.
     *
     * @param file Input file stream positioned after reading the facet normal line.
     * @param headerLine The facet normal line read before calling this function.
     * @return Triangle The parsed triangle with vertices and normal.
     */
    Triangle readFacet(std::ifstream& file, const std::string_view& headerLine);
};
