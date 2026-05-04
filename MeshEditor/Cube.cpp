#include "Cube.hpp"
#include "STLParser.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <ranges>

const std::string& Cube::getName() const
{
	static const std::string name = "Cube";
	return name;
}

int Cube::execute(const std::map<std::string, std::string>& args)
{
	const auto itL = args.find("L");
	const auto itOrigin = args.find("origin");
	const auto itFile = args.find("filepath");

	if (itL == args.end() || itOrigin == args.end() || itFile == args.end() )
		return 3;

    try 
	{
        length = std::stod(itL->second);
    }
    catch (const std::exception&) 
	{
        return 3; 
    }

    if (equal(length, 0.0) || length < 0.0)
        return 1;

    try 
	{
		origin = parseOrigin(itOrigin->second);
    }
    catch (const std::exception&) 
	{
        return 3; 
    }

    filepath = itFile->second;

    TriangleSoup soup = createCube();

	try
	{
		STLParser parser;
		parser.write(soup, filepath);
	}
	catch (const std::exception&)
	{
		return 2;
	}

    return 0;
}

TriangleSoup Cube::createCube() const
{
	TriangleSoup soup;
	const auto [x, y, z] = origin;
	const double L = length;

	const Vec v[8] =
	{
		{x, y, z}, //0
		{x + L, y, z}, //1
		{x + L, y + L, z}, //2
		{x, y + L, z}, //3
		{x, y, z + L}, //4
		{x + L, y, z + L}, //5
		{x + L, y + L, z + L}, //6
		{x, y + L, z + L} //7
	};

	auto addFace = [&soup, &v](int a, int b, int c, int d)
		{
			const Vec normal = calculateNormal(v[a], v[b], v[c]);
			soup.emplace_back(Triangle{ v[a], v[b], v[c], normal });
			soup.emplace_back(Triangle{ v[a], v[c], v[d], normal });
		};

	addFace(0, 3, 2, 1); // Bottom face 
	addFace(4, 5, 6, 7); // Top face 
	addFace(0, 1, 5, 4); // Front face 
	addFace(2, 3, 7, 6); // Back face 
	addFace(1, 2, 6, 5); // Right face 
	addFace(3, 0, 4, 7); // Left face 

	return soup;
}