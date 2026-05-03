#include "Sphere.hpp"
#include "STLParser.hpp"

#include <sstream>
#include <cmath>
#include <numbers>

const std::string& Sphere::getName() const
{
    static const std::string name = "Sphere";
    return name;
}

int Sphere::execute(const std::map<std::string, std::string>& args)
{
    const auto itRadius = args.find("R");
    const auto itOrigin = args.find("origin");
    const auto itSlices = args.find("slices");
    const auto itRings = args.find("rings");
    const auto itFile = args.find("filepath");

    if ( itRadius == args.end() || itOrigin == args.end() || itSlices == args.end() || itRings == args.end() || itFile == args.end() )
        return 4;

    try
    {
        radius = std::stod(itRadius->second);
    }
    catch (const std::exception&)
    {
        return 4;
    }

    if (equal(radius, 0.0) || radius < 0.0)
        return 1;

    try
    {
        origin = parseOrigin(itOrigin->second);
    }
    catch(const std::exception&)
    {
        return 4;
    }

    try
    {
        nSlices = std::stoi(itSlices->second);
        nRings = std::stoi(itRings->second);
    }
    catch (const std::exception&)
    {
        return 4;
    }

    if (nSlices <= 2 || nRings <= 1)
        return 2;

    filepath = itFile->second;

    TriangleSoup soup = createSphere();

    try
    {
        STLParser parser;
        parser.write(soup, filepath);
    }
    catch (const std::exception&)
    {
        return 3;
    }

    return 0;
}

TriangleSoup Sphere::createSphere() const
{
    TriangleSoup soup;
    constexpr double pi = std::numbers::pi;

    std::vector<Vec> vertices((nRings - 1) * (nSlices + 1) + 2);

    auto vertexIndex = [this](int ring, int slice) 
        {
        if (ring == 0) return 0; 
        if (ring == nRings) return (nRings - 1) * (nSlices + 1) + 1; 
        return 1 + (ring - 1) * (nSlices + 1) + slice;
        };

    vertices[0] = { origin.x, origin.y, origin.z + radius };

    for (int ring = 1; ring < nRings; ++ring)
    {
        double theta = pi * ring / nRings;
        for (int slice = 0; slice <= nSlices; ++slice)
        {
            double phi = 2 * pi * slice / nSlices;
            vertices[vertexIndex(ring, slice)] = sphericalToCartesian(theta, phi);
        }
    }

    vertices.back() = { origin.x, origin.y, origin.z - radius };

    for (int slice = 0; slice < nSlices; ++slice)
    {
        soup.emplace_back(Triangle{
            vertices[vertexIndex(0, 0)],
            vertices[vertexIndex(1, slice)],
            vertices[vertexIndex(1, slice + 1)],
            calculateNormal(vertices[vertexIndex(0, 0)], vertices[vertexIndex(1, slice)], vertices[vertexIndex(1, slice + 1)])
            });

        for (int ring = 1; ring < nRings - 1; ++ring)
        {
            const Vec v0 = vertices[vertexIndex(ring, slice)];
            const Vec v1 = vertices[vertexIndex(ring, slice + 1)];
            const Vec v2 = vertices[vertexIndex(ring + 1, slice + 1)];
            const Vec v3 = vertices[vertexIndex(ring + 1, slice)];

            const Vec normal1 = calculateNormal(v0, v2, v1);
            const Vec normal2 = calculateNormal(v0, v3, v2);

            soup.emplace_back(Triangle{ v0, v2, v1, normal1 });
            soup.emplace_back(Triangle{ v0, v3, v2, normal2 });
        }

        soup.emplace_back(Triangle{
            vertices[vertexIndex(nRings, 0)],
            vertices[vertexIndex(nRings - 1, slice + 1)],
            vertices[vertexIndex(nRings - 1, slice)],
            calculateNormal(vertices[vertexIndex(nRings, 0)], vertices[vertexIndex(nRings - 1, slice + 1)], vertices[vertexIndex(nRings - 1, slice)])
            });
    }

    return soup;
}


Vec Sphere::sphericalToCartesian(double theta, double phi) const 
{
    const double x = radius * std::sin(theta) * std::cos(phi) + origin.x;
    const double y = radius * std::sin(theta) * std::sin(phi) + origin.y;
    const double z = radius * std::cos(theta) + origin.z;
    return { x, y, z };
}
