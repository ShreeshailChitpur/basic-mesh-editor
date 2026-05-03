#include "STLParser.hpp"

#include <string_view>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>

static std::string_view trim(std::string_view str) 
{
    const auto begin = str.find_first_not_of(" \t\n\r\f\v");
    if (begin == std::string_view::npos)
        return {};

    const auto end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(begin, end - begin + 1);
}

Vec STLParser::parseVertexLine(const std::string_view& line)
{
    std::istringstream iss(line.data());
    std::string keyword;
    double x, y, z;
    iss >> keyword >> x >> y >> z;
    return { x, y, z };
}

Vec parseOrigin(const std::string& str)
{
    Vec v{};
    std::stringstream ss(str);
    std::string value;

    if (std::getline(ss, value, ',')) v.x = std::stod(value);
    if (std::getline(ss, value, ',')) v.y = std::stod(value);
    if (std::getline(ss, value, ',')) v.z = std::stod(value);

    return v;
}

Vec STLParser::parseNormalLine(const std::string_view& line)
{
    std::istringstream iss(line.data());
    std::string facet, normal;
    double nx, ny, nz;
    iss >> facet >> normal >> nx >> ny >> nz;
    return { nx, ny, nz };
}

Triangle STLParser::readFacet(std::ifstream& file, const std::string_view& headerLine) 
{
    Triangle triangle;

    triangle.normal = parseNormalLine(headerLine);

    std::string line;
    int vertexIndex = 0;

    while (vertexIndex < 3 && std::getline(file, line)) 
    {
        line = std::string(trim(line)); 

        if (line.find("vertex") == 0) 
        {
            Vec v = parseVertexLine(line);
            if (vertexIndex == 0) triangle.A = v;
            else if (vertexIndex == 1) triangle.B = v;
            else if (vertexIndex == 2) triangle.C = v;
            vertexIndex++;
        }
    }

    const bool normalIsZero = equal(triangle.normal.x, 0.0) && equal(triangle.normal.y, 0.0) && equal(triangle.normal.z, 0.0);
    
    if (normalIsZero)
        triangle.normal = calculateNormal(triangle.A, triangle.B, triangle.C);

    return triangle;
}

TriangleSoup STLParser::read(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        throw std::runtime_error("Failed to open file: " + filename);
    }

    TriangleSoup soup;
    std::string line;

    while (std::getline(file, line)) 
    {
        if (line.find("facet normal") != std::string::npos) 
            soup.push_back(readFacet(file, trim(line)));
    }

    return soup;
}

void STLParser::write(const TriangleSoup& triangleSoup, const std::string& filename)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Cannot write to file" << filename << "\n";
        return;
    }

    file << "solid mesh\n";

    for (const auto& triangle : triangleSoup)
    {
        Vec normal = calculateNormal(triangle.A, triangle.B, triangle.C);

        file << " facet normal " << normal.x << " " << normal.y << " " << normal.z << "\n";
        file << "    outer loop\n";

        file << "      vertex " << triangle.A.x << " " << triangle.A.y << " " << triangle.A.z << "\n";
        file << "      vertex " << triangle.B.x << " " << triangle.B.y << " " << triangle.B.z << "\n";
        file << "      vertex " << triangle.C.x << " " << triangle.C.y << " " << triangle.C.z << "\n";

        file << "    endloop\n";
        file << "  endfacet\n";
    }

    file << "endsolid mesh\n";
}
