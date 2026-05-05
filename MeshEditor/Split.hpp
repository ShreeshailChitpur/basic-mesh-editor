#pragma once

#include "Command.hpp"
#include "Geometry.hpp"

#include <vector>
#include <map>
#include <string>

/**
 * @brief Helper structure storing signed distances of triangle vertices to a plane.
 */
struct TriangleDistances
{
    double dA, dB, dC;
};

/**
 * @class Split
 * @brief Splits a 3D STL mesh into two parts using a cutting plane.
 *
 * The Split command takes:
 * - An input STL mesh.
 * - A splitting plane defined by an origin point and a direction (normal).
 *
 * It outputs two STL files:
 * - One containing triangles on the positive side of the plane.
 * - One containing triangles on the negative side of the plane.
 *
 * The class:
 * - Computes signed distances of vertices to the splitting plane.
 * - Clips triangles intersecting the plane.
 * - Reconstructs and tessellates planar "cap" faces to close open holes.
 */
class Split : public Command
{
public:
    /**
     * @brief Returns the command's unique name ("Split").
     *
     * @return Name of the command.
     */
    const std::string& getName() const override;

    /**
     * @brief Executes the split operation.
     *
     * Expected arguments:
     * - `"input"`: Path to input STL file.
     * - `"output1"`: Path to output STL mesh above the plane.
     * - `"output2"`: Path to output STL mesh below the plane.
     * - `"origin"`: Comma-separated plane origin vector.
     * - `"direction"`: Comma-separated plane normal vector.
     *
     * Error return codes:
     * - `0` : Success.
     * - `1` : normal length <= 0.
     * - `2` : File I/O error (unable to read or write).
     * - `3` : Invalid or missing arguments.
     * - `4` : Split failed due to plane doesn’t intersect the original mesh.
     *
     * @param args Map of argument keys to string values.
     * @return Status code.
     */
    int execute(const std::map<std::string, std::string>& args) override;

private:
    /**
     * @brief Computes the signed distance of a point to a plane.
     *
     * Positive result: point is "above".
     * Negative result: point is "below".
     *
     * @param point Point to evaluate.
     * @param origin Origin point of plane.
     * @param direction Normal vector of plane (must be normalized).
     * @return Signed distance to plane.
     */
    double signedDistanceToPlane(const Vec point, const Vec origin, const Vec direction) const;

    /**
     * @brief Computes signed distances for all three vertices of a triangle to a plane.
     *
     * @param tri Triangle to evaluate.
     * @param origin Plane origin.
     * @param direction Plane normal (must be normalized).
     * @return Structure containing distances for vertices A, B, and C.
     */
    TriangleDistances computeTriangleDistances(const Triangle& tri, const Vec origin, const Vec direction) const;

    /**
     * @brief Clips a triangle that intersects the plane into one or two polygons.
     *
     * Depending on vertex classification, this may:
     * - Keep the triangle fully above.
     * - Keep the triangle fully below.
     * - Split the triangle into one above-polygon and one below-polygon.
     *
     * @param tri Input triangle.
     * @param distances Precomputed signed distances for vertices A, B, and C.
     * @return Pair of triangle soups: first contains geometry above the plane, second contains geometry below.
     */
    std::pair<TriangleSoup, TriangleSoup> clipTriangle(const Triangle& tri, const TriangleDistances& distances);

    /**
     * @brief Detects and tessellates planar holes created by splitting.
     *
     * After cutting the mesh, open edges along the cutting plane are gathered,
     * looped into polygons, and triangulated to seal the mesh on both sides.
     *
     * @param input  Original mesh before splitting.
     * @param origin Plane origin.
     * @param direction Plane normal.
     * @param above Mesh above the plane (receives cap triangles).
     * @param below Mesh below the plane (receives cap triangles).
     */
    void tessellateHoles(const TriangleSoup& input, const Vec origin, const Vec direction, TriangleSoup& above, TriangleSoup& below);

    /**
     * @brief Extracts a merged polygon representing the hole boundary.
     *
     * Useful for debugging or tools that need the raw boundary loop.
     *
     * @param input Input mesh.
     * @param origin Plane origin.
     * @param direction Plane normal.
     * @return Vector of ordered vertices forming the hole, or empty vector if no hole exists.
     */
    std::vector<Vec> extractHolePolygon(const TriangleSoup& input, const Vec origin, const Vec direction);

    /**
    * @brief Ear-clipping triangulation of an arbitrary simple polygon.
    *
    * The polygon is assumed to lie in a plane. A projection direction
    * (the plane normal) is used for convex/concave tests.
    *
    * @param polygon The list of vertices in order.
    * @param normal Plane normal defining orientation.
    * @return Triangulated result as a triangle soup.
    */
    TriangleSoup earClipTriangulate(const std::vector<Vec>& polygon, const Vec normal);

private:
    /**
     * @brief Helper structure representing a region with an outer boundary and holes.
     */
    struct LoopRegion
    {
        size_t outerIdx;
        std::vector<size_t> holeIndices;
    };

    /**
     * @brief Helper structure containing all loops extracted from a plane intersection.
     */
    struct PlaneLoopsData
    {
        std::vector<std::vector<Vec>> allLoops;
        std::vector<LoopRegion> regions;
        Vec U, V;  
        Vec origin;
    };

    /**
     * @brief Extracts and classifies all loops (outer boundaries and holes) from plane intersection.
     *
     * This helper method performs edge extraction, loop assembly, and region classification
     * to identify outer boundaries and their nested holes. Supports multiple separate regions
     * and multiple holes per region (e.g., torus geometry).
     *
     * @param inputMesh Input mesh to intersect with plane.
     * @param origin Plane origin.
     * @param direction Plane normal.
     * @return Structure containing all loops and their hierarchical relationships, or empty structure if no intersection.
     */
    PlaneLoopsData extractPlaneLoops(const TriangleSoup& inputMesh, const Vec origin, const Vec direction);

    /**
     * @brief Merges an outer loop with multiple holes into a single polygon.
     *
     * Uses bridge vertices to connect holes to the outer boundary, creating
     * a single simple polygon suitable for ear-clipping triangulation.
     *
     * @param outer Outer boundary loop (CCW).
     * @param holes Vector of hole loops (CW).
     * @param U 2D basis vector for plane.
     * @param V 2D basis vector for plane.
     * @param origin Plane origin.
     * @return Merged polygon with all holes connected.
     */
    std::vector<Vec> mergeLoopWithHoles(const std::vector<Vec>& outer, const std::vector<std::vector<Vec>>& holes, Vec U, Vec V, Vec origin);
};
