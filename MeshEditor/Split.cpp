#include "Split.hpp"
#include "STLParser.hpp"

#include <cmath>
#include <algorithm>
#include <set>
#include <numeric>

namespace
{
    Vec interpolate(const Vec v1, const Vec v2, double t)
    {
        return add(v1, scale(v2 - v1, t));
    }

    struct Edge 
    { 
        Vec s, e; 
    };
}

TriangleSoup Split::earClipTriangulate(const std::vector<Vec>& polygon, const Vec normal)
{
    TriangleSoup triangles;

    if (polygon.size() < 3) 
        return triangles;

    auto isPolygonCounterClockwise = [&polygon, &normal]() -> bool 
        {
        if (polygon.size() < 3)
            return true;

        double signedArea = 0.0;
        for (size_t i = 0; i < polygon.size(); ++i)
        {
            const Vec& a = polygon[i];
            const Vec& b = polygon[(i + 1) % polygon.size()];
            const Vec ab = b - a;
            const Vec crossVec = cross(a, ab);
            signedArea += dot(crossVec, normal);
        }
        return signedArea > 0.0;
        };

    std::vector<Vec> workingPolygon = polygon;
    if (!isPolygonCounterClockwise())
    {
        std::reverse(workingPolygon.begin(), workingPolygon.end());
    }

    auto isPointInTriangle = [](const Vec& p, const Vec& a, const Vec& b, const Vec& c) -> bool 
        {
        const Vec v0{ c.x - a.x, c.y - a.y, c.z - a.z };
        const Vec v1{ b.x - a.x, b.y - a.y, b.z - a.z };
        const Vec v2{ p.x - a.x, p.y - a.y, p.z - a.z };

        const double dot00 = dot(v0, v0), dot01 = dot(v0, v1), dot02 = dot(v0, v2);
        const double dot11 = dot(v1, v1), dot12 = dot(v1, v2);
        const double denom = dot00 * dot11 - dot01 * dot01;

        if (std::fabs(denom) < 1e-10)
            return false;

        const double inv = 1.0 / denom;
        const double u = (dot11 * dot02 - dot01 * dot12) * inv;
        const double v = (dot00 * dot12 - dot01 * dot02) * inv;
        return (u > 0) && (v > 0) && (u + v < 1);
        };

    auto isEar = [&workingPolygon, &normal, &isPointInTriangle](size_t aIdx, size_t bIdx, size_t cIdx) -> bool 
        {
        const Vec& a = workingPolygon[aIdx];
        const Vec& b = workingPolygon[bIdx];
        const Vec& c = workingPolygon[cIdx];

        const Vec ab = b - a;
        const Vec bc = c - b;
        const Vec crossProduct = cross(ab, bc);
        const double dotProduct = dot(crossProduct, normal);

        if (dotProduct <= 0) return false;

        for (size_t i = 0; i < workingPolygon.size(); ++i)
        {
            if (i == aIdx || i == bIdx || i == cIdx)
                continue;
            if (isPointInTriangle(workingPolygon[i], a, b, c))
                return false;
        }
        return true;
        };

    std::vector<size_t> vertexIndices(workingPolygon.size());
    std::iota(vertexIndices.begin(), vertexIndices.end(), 0);

    while (vertexIndices.size() > 3)
    {
        bool earFound = false;
        for (size_t i = 0; i < vertexIndices.size(); ++i)
        {
            const size_t aIdx = vertexIndices[(i + vertexIndices.size() - 1) % vertexIndices.size()];
            const size_t bIdx = vertexIndices[i];
            const size_t cIdx = vertexIndices[(i + 1) % vertexIndices.size()];

            if (isEar(aIdx, bIdx, cIdx))
            {
                triangles.push_back(Triangle{ workingPolygon[aIdx], workingPolygon[bIdx], workingPolygon[cIdx], normal });
                vertexIndices.erase(vertexIndices.begin() + i);
                earFound = true;
                break;
            }
        }

        if (!earFound)
        {
            for (size_t i = 1; i + 1 < vertexIndices.size(); ++i)
                triangles.push_back(Triangle{ workingPolygon[vertexIndices[0]], workingPolygon[vertexIndices[i]], workingPolygon[vertexIndices[i + 1]], normal });
            return triangles;
        }
    }

    if (vertexIndices.size() == 3)
        triangles.push_back(Triangle{ workingPolygon[vertexIndices[0]], workingPolygon[vertexIndices[1]], workingPolygon[vertexIndices[2]], normal });

    return triangles;
}

Split::PlaneLoopsData Split::extractPlaneLoops(const TriangleSoup& inputMesh, const Vec origin, const Vec direction)
{
    std::vector<Edge> planeEdges;

    for (const Triangle& tri : inputMesh)
    {
        const TriangleDistances distances = computeTriangleDistances(tri, origin, direction);
        const double dA = distances.dA, dB = distances.dB, dC = distances.dC;
        std::vector<Vec> intersectionPoints;

        auto computeEdgeIntersection = [&intersectionPoints](const Vec& a, const Vec& b, double distanceA, double distanceB)
            {
                if (std::fabs(distanceA) < eps)
                {
                    bool shouldAddPoint = true;
                    for (auto& existingPoint : intersectionPoints)
                    {
                        if (existingPoint == a)
                        {
                            shouldAddPoint = false;
                            break;
                        }
                    }
                    if (shouldAddPoint) 
                        intersectionPoints.push_back(a);
                }
                if ((distanceA > eps && distanceB < -eps) || (distanceA < -eps && distanceB > eps))
                {
                    double t = distanceA / (distanceA - distanceB);
                    intersectionPoints.push_back(interpolate(a, b, t));
                }
            };

        computeEdgeIntersection(tri.A, tri.B, dA, dB);
        computeEdgeIntersection(tri.B, tri.C, dB, dC);
        computeEdgeIntersection(tri.C, tri.A, dC, dA);

        if (intersectionPoints.size() == 2)
            planeEdges.push_back(Edge{ intersectionPoints[0], intersectionPoints[1] });
    }

    if (planeEdges.empty())
        return PlaneLoopsData{};

    std::vector<bool> edgeUsed(planeEdges.size(), false);
    std::vector<std::vector<Vec>> allLoops;

    for (size_t startEdgeIdx = 0; startEdgeIdx < planeEdges.size(); ++startEdgeIdx)
    {
        if (edgeUsed[startEdgeIdx])
            continue;

        std::vector<Vec> loop;
        loop.push_back(planeEdges[startEdgeIdx].s);
        loop.push_back(planeEdges[startEdgeIdx].e);
        edgeUsed[startEdgeIdx] = true;
        Vec currentVertex = planeEdges[startEdgeIdx].e;

        while (true)
        {
            bool foundConnection = false;
            for (size_t edgeIdx = 0; edgeIdx < planeEdges.size(); ++edgeIdx)
            {
                if (edgeUsed[edgeIdx])
                    continue;
                Edge& edge = planeEdges[edgeIdx];

                if (currentVertex == edge.s)
                {
                    loop.push_back(edge.e);
                    currentVertex = edge.e;
                    edgeUsed[edgeIdx] = true;
                    foundConnection = true;
                }
                else if (currentVertex == edge.e)
                {
                    loop.push_back(edge.s);
                    currentVertex = edge.s;
                    edgeUsed[edgeIdx] = true;
                    foundConnection = true;
                }

                if (foundConnection) 
                    break;
            }

            if (!foundConnection)
                break;
            if (currentVertex == loop.front())
                break;
        }

        if (loop.size() > 2 && (loop.front() == loop.back()))
            loop.pop_back();

        if (loop.size() >= 3)
            allLoops.push_back(loop);
    }

    if (allLoops.empty())
        return PlaneLoopsData{};

    const Vec planeNormal = direction;
    const Vec auxiliaryVector = std::fabs(planeNormal.x) > 0.9 ? Vec{ 0,1,0 } : Vec{ 1,0,0 };
    const Vec basisU = normalize(cross(auxiliaryVector, planeNormal));
    const Vec basisV = normalize(cross(planeNormal, basisU));

    auto signedArea = [&origin, &basisU, &basisV](const std::vector<Vec>& loop)
        {
            double area = 0;
            const size_t numVertices = loop.size();
            for (size_t i = 0, j = numVertices - 1; i < numVertices; j = i++)
            {
                const Vec vertexI = loop[i];
                const Vec vertexJ = loop[j];
                const double xi = dot(vertexI - origin, basisU), yi = dot(vertexI - origin, basisV);
                const double xj = dot(vertexJ - origin, basisU), yj = dot(vertexJ - origin, basisV);
                area += (xj * yi - xi * yj);
            }
            return 0.5 * area;
        };

    auto pointInPolygon = [&origin, &basisU, &basisV](const Vec& testPoint, const std::vector<Vec>& polygon) -> bool
        {
            const double pointX = dot(testPoint - origin, basisU);
            const double pointY = dot(testPoint - origin, basisV);
            int windingNumber = 0;

            for (size_t i = 0; i < polygon.size(); ++i)
            {
                size_t j = (i + 1) % polygon.size();
                const double xi = dot(polygon[i] - origin, basisU);
                const double yi = dot(polygon[i] - origin, basisV);
                const double xj = dot(polygon[j] - origin, basisU);
                const double yj = dot(polygon[j] - origin, basisV);

                if (yi <= pointY)
                {
                    if (yj > pointY)
                    {
                        const double crossProduct = (xj - xi) * (pointY - yi) - (pointX - xi) * (yj - yi);
                        if (crossProduct > 0)
                            windingNumber++;
                    }
                }
                else
                {
                    if (yj <= pointY)
                    {
                        const double crossProduct = (xj - xi) * (pointY - yi) - (pointX - xi) * (yj - yi);
                        if (crossProduct < 0)
                            windingNumber--;
                    }
                }
            }
            return windingNumber != 0;
        };

    std::vector<LoopRegion> regions;
    std::vector<bool> loopProcessed(allLoops.size(), false);

    std::vector<std::pair<double, size_t>> loopsByArea;
    for (size_t i = 0; i < allLoops.size(); ++i)
    {
        loopsByArea.push_back({ std::fabs(signedArea(allLoops[i])), i });
    }

    std::sort(loopsByArea.begin(), loopsByArea.end(), std::greater<>());

    for (const auto& [area, loopIdx] : loopsByArea)
    {
        if (loopProcessed[loopIdx])
            continue;

        LoopRegion region;
        region.outerIdx = loopIdx;
        loopProcessed[loopIdx] = true;

        for (size_t candidateIdx = 0; candidateIdx < allLoops.size(); ++candidateIdx)
        {
            if (loopProcessed[candidateIdx] || candidateIdx == loopIdx)
                continue;

            if (!allLoops[candidateIdx].empty() && pointInPolygon(allLoops[candidateIdx][0], allLoops[loopIdx]))
            {
                region.holeIndices.push_back(candidateIdx);
                loopProcessed[candidateIdx] = true;
            }
        }
        regions.push_back(region);
    }

    PlaneLoopsData result;
    result.allLoops = std::move(allLoops);
    result.regions = std::move(regions);
    result.U = basisU;
    result.V = basisV;
    result.origin = origin;

    return result;
}

std::vector<Vec> Split::mergeLoopWithHoles(const std::vector<Vec>& outer, const std::vector<std::vector<Vec>>& holes, Vec U, Vec V, Vec origin)
{
    if (holes.empty())
        return outer;

    auto signedArea = [&origin, &U, &V](const std::vector<Vec>& loop)
        {
            double area = 0;
            const size_t numVertices = loop.size();
            for (size_t i = 0, j = numVertices - 1; i < numVertices; j = i++)
            {
                const Vec vertexI = loop[i];
                const Vec vertexJ = loop[j];
                const double xi = dot(vertexI - origin, U), yi = dot(vertexI - origin, V);
                const double xj = dot(vertexJ - origin, U), yj = dot(vertexJ - origin, V);
                area += (xj * yi - xi * yj);
            }
            return 0.5 * area;
        };

    std::vector<Vec> result = outer;
    if (signedArea(result) < 0)
        std::reverse(result.begin(), result.end());

    for (const auto& holeLoop : holes)
    {
        std::vector<Vec> hole = holeLoop;
        if (signedArea(hole) > 0)
            std::reverse(hole.begin(), hole.end());

        double minDistance = 1e300;
        size_t outerVertexIdx = 0, holeVertexIdx = 0;
        for (size_t i = 0; i < result.size(); ++i)
        {
            for (size_t j = 0; j < hole.size(); ++j)
            {
                const double dist = length(result[i] - hole[j]);
                if (dist < minDistance)
                {
                    minDistance = dist;
                    outerVertexIdx = i;
                    holeVertexIdx = j;
                }
            }
        }

        std::vector<Vec> merged;
        merged.push_back(result[outerVertexIdx]);

        for (size_t k = 1; k < result.size(); ++k)
            merged.push_back(result[(outerVertexIdx + k) % result.size()]);

        merged.push_back(result[outerVertexIdx]);
        merged.push_back(hole[holeVertexIdx]);

        for (size_t k = 1; k < hole.size(); ++k)
            merged.push_back(hole[(holeVertexIdx + k) % hole.size()]);

        merged.push_back(hole[holeVertexIdx]);
        merged.push_back(result[outerVertexIdx]);

        std::vector<Vec> cleaned;
        cleaned.reserve(merged.size());
        for (size_t i = 0; i < merged.size(); ++i)
        {
            if (i == 0 || !(merged[i] == merged[i - 1]))
                cleaned.push_back(merged[i]);
        }

        result = std::move(cleaned);
    }

    return result;
}

const std::string& Split::getName() const
{
    static const std::string commandName = "Split";
    return commandName;
}

int Split::execute(const std::map<std::string, std::string>& args)
{
    const auto itInput = args.find("input");
    const auto itOutput1 = args.find("output1");
    const auto itOutput2 = args.find("output2");
    const auto itOrigin = args.find("origin");
    const auto itDirection = args.find("direction");

    if (itInput == args.end() || itOutput1 == args.end() || itOutput2 == args.end() || itOrigin == args.end() || itDirection == args.end())
        return 3;

    Vec origin, direction;

    try
    {
        origin = parseOrigin(itOrigin->second);
        direction = normalize(parseOrigin(itDirection->second));
    }
    catch (const std::exception&)
    {
        return 3;
    }

    const std::string inputFile = itInput->second;
    const std::string outputFile1 = itOutput1->second;
    const std::string outputFile2 = itOutput2->second;

    if (direction == Vec{ 0.0, 0.0, 0.0 })
        return 1;

    STLParser parser;
    TriangleSoup soup;

    try
    {
        soup = parser.read(inputFile);
    }
    catch (const std::exception&)
    {
        return 2;
    }

    TriangleSoup meshAbove, meshBelow;

    for (const Triangle& tri : soup)
    {
        const TriangleDistances distances = computeTriangleDistances(tri, origin, direction);

        const bool aAbove = distances.dA > eps, bAbove = distances.dB > eps, cAbove = distances.dC > eps;
        const bool aBelow = distances.dA < -eps, bBelow = distances.dB < -eps, cBelow = distances.dC < -eps;

        const int numAbove = (aAbove)+(bAbove)+(cAbove);
        const int numBelow = (aBelow)+(bBelow)+(cBelow);

        if (numAbove == 3)
        {
            meshAbove.push_back(tri);
        }
        else if (numBelow == 3)
        {
            meshBelow.push_back(tri);
        }
        else if (numAbove == 0 && numBelow == 0)
        {
            meshAbove.push_back(tri);
            meshBelow.push_back(tri);
        }
        else
        {
            const auto [above, below] = clipTriangle(tri, distances);
            meshAbove.insert(meshAbove.end(), above.begin(), above.end());
            meshBelow.insert(meshBelow.end(), below.begin(), below.end());
        }
    }

    tessellateHoles(soup, origin, direction, meshAbove, meshBelow);

    if (meshAbove.empty() || meshBelow.empty())
        return 4;

    try
    {
        parser.write(meshAbove, outputFile1);
        parser.write(meshBelow, outputFile2);
    }
    catch (const std::exception&)
    {
        return 2;
    }

    return 0;
}

double Split::signedDistanceToPlane(const Vec p, const Vec origin, const Vec direction) const
{
    return (p.x - origin.x) * direction.x + (p.y - origin.y) * direction.y + (p.z - origin.z) * direction.z;
}

TriangleDistances Split::computeTriangleDistances(const Triangle& tri, const Vec origin, const Vec direction) const
{
    return TriangleDistances{
        signedDistanceToPlane(tri.A, origin, direction),
        signedDistanceToPlane(tri.B, origin, direction),
        signedDistanceToPlane(tri.C, origin, direction)
    };
}

std::pair<TriangleSoup, TriangleSoup> Split::clipTriangle(const Triangle& tri, const TriangleDistances& distances)
{
    const double dA = distances.dA;
    const double dB = distances.dB;
    const double dC = distances.dC;

    constexpr auto classify = [](double distance) constexpr -> int
        {
            return (distance > eps) ? 1 : ((distance < -eps) ? -1 : 0);
        };

    const int signA = classify(dA), signB = classify(dB), signC = classify(dC);

    const int aboveCount = (signA == 1) + (signB == 1) + (signC == 1);
    const int belowCount = (signA == -1) + (signB == -1) + (signC == -1);
    const int onPlaneCount = (signA == 0) + (signB == 0) + (signC == 0);

    TriangleSoup meshAbove, meshBelow;

    if (aboveCount == 0 || belowCount == 0)
    {
        if (aboveCount + onPlaneCount == 3) 
            meshAbove.push_back(tri);
        if (belowCount + onPlaneCount == 3) 
            meshBelow.push_back(tri);
        return { meshAbove, meshBelow };
    }

    std::vector<Vec> abovePoints, belowPoints;

    auto processEdge = [&abovePoints, &belowPoints](const Vec a, const Vec b, double distanceA, double distanceB, int signA, int signB) 
        {
        if (signA == 1)
            abovePoints.push_back(a);
        if (signA == -1)
            belowPoints.push_back(a);

        if (signA * signB < 0)
        {
            const double t = distanceA / (distanceA - distanceB);
            const Vec intersectionPoint = interpolate(a, b, t);
            abovePoints.push_back(intersectionPoint);
            belowPoints.push_back(intersectionPoint);
        }
        else if (signB == 0)
        {
            if (signA == 1)
                abovePoints.push_back(b);
            if (signA == -1)
                belowPoints.push_back(b);
        }
        };

    processEdge(tri.A, tri.B, dA, dB, signA, signB);
    processEdge(tri.B, tri.C, dB, dC, signB, signC);
    processEdge(tri.C, tri.A, dC, dA, signC, signA);

    if (abovePoints.size() >= 3)
    {
        for (size_t i = 1; i + 1 < abovePoints.size(); ++i)
        {
            meshAbove.push_back(Triangle{ abovePoints[0], abovePoints[i], abovePoints[i + 1], tri.normal });
        }
    }

    if (belowPoints.size() >= 3)
    {
        for (size_t i = 1; i + 1 < belowPoints.size(); ++i)
        {
            meshBelow.push_back(Triangle{ belowPoints[0], belowPoints[i], belowPoints[i + 1], tri.normal });
        }
    }

    return { meshAbove, meshBelow };
}

void Split::tessellateHoles(const TriangleSoup& inputMesh, const Vec origin, const Vec direction, TriangleSoup& meshAbove, TriangleSoup& meshBelow)
{
    auto loopsData = extractPlaneLoops(inputMesh, origin, direction);
    if (loopsData.regions.empty())
        return;

    const Vec planeNormal = direction;

    for (const LoopRegion& region : loopsData.regions)
    {
        std::vector<Vec> outerLoop = loopsData.allLoops[region.outerIdx];

        std::vector<std::vector<Vec>> holes;
        for (size_t holeIdx : region.holeIndices)
        {
            holes.push_back(loopsData.allLoops[holeIdx]);
        }

        std::vector<Vec> mergedPolygon = mergeLoopWithHoles(outerLoop, holes, loopsData.U, loopsData.V, loopsData.origin);

        const TriangleSoup capTriangles = earClipTriangulate(mergedPolygon, planeNormal);

        for (const Triangle& triangle : capTriangles)
        {
            Triangle aboveTriangle = triangle;
            aboveTriangle.normal = planeNormal;
            meshBelow.push_back(aboveTriangle);

            Triangle belowTriangle{ triangle.A, triangle.C, triangle.B, scale(planeNormal, -1) };
            meshAbove.push_back(belowTriangle);
        }
    }
}

std::vector<Vec> Split::extractHolePolygon(const TriangleSoup& inputMesh, const Vec origin, const Vec direction)
{
    auto loopsData = extractPlaneLoops(inputMesh, origin, direction);
    if (loopsData.regions.empty())
        return {};

    const LoopRegion& firstRegion = loopsData.regions[0];
    std::vector<Vec> outerLoop = loopsData.allLoops[firstRegion.outerIdx];

    std::vector<std::vector<Vec>> holes;
    for (size_t holeIdx : firstRegion.holeIndices)
    {
        holes.push_back(loopsData.allLoops[holeIdx]);
    }

    return mergeLoopWithHoles(outerLoop, holes, loopsData.U, loopsData.V, loopsData.origin);
}
