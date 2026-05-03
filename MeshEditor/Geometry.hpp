#pragma once

#include <vector>
#include <cmath>
#include <string>

/**
 * @file Mesh.hpp
 * @brief Common mesh and geometry utilities for 3D operations.
 *
 * This header contains fundamental structures and functions for working with
 * 3D meshes, vectors, and geometric calculations used across the application.
 */

/**
 * @brief Tolerance value for floating-point comparisons.
 */
constexpr double eps = 1e-8;

/**
 * @struct Vec
 * @brief Represents a 3D vector or point in Cartesian coordinates.
 */
struct Vec 
{
    double x, y, z;

    bool operator==(const Vec& other) const
    {
        return std::fabs(x - other.x) < eps
            && std::fabs(y - other.y) < eps
            && std::fabs(z - other.z) < eps;
    }
};

/**
 * @struct Vec2
 * @brief Represents a 2D point or vector with double precision.
 *
 * This helper structure is used for operations that only require
 * planar coordinates (e.g., ear clipping operations in 2D projection).
 */
struct Vec2
{
    double x, y;
};

/**
 * @struct Triangle
 * @brief Represents a triangle in 3D space with an associated normal vector.
 *
 * Each triangle consists of three vertices (A, B, C) and a normal vector.
 * The normal is used in rendering and defines the triangle's orientation.
 */
struct Triangle
{
    Vec A, B, C; 
    Vec normal; 
};

/**
 * @typedef TriangleSoup
 * @brief A collection of triangles forming a mesh.
 *
 * This alias is used to describe a list of triangles that collectively define
 * a 3D shape or object parsed from or written to an STL file.
 */
using TriangleSoup = std::vector<Triangle>;

// ============================================================================
// Vector Operations
// ============================================================================

/**
 * @brief Subtracts two vectors.
 * @param a First vector.
 * @param b Second vector.
 * @return Resulting vector (a - b).
 */
inline Vec operator-(Vec a, Vec b) 
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

/**
 * @brief Adds two vectors element-wise.
 *
 * @param a First vector.
 * @param b Second vector.
 * @return Vector sum (a + b).
 */
inline Vec add(const Vec a, const Vec b)
{
    return { a.x + b.x, a.y + b.y, a.z + b.z };
}

/**
 * @brief Scales a vector by a scalar multiplier.
 *
 * @param v Input vector.
 * @param s Scalar scale factor.
 * @return A new vector equal to v * s.
 */
inline Vec scale(const Vec v, double s)
{
    return { v.x * s, v.y * s, v.z * s };
}

/**
 * @brief Computes the dot product of two 3D vectors.
 *
 * @param a First input vector.
 * @param b Second input vector.
 * @return Dot product value (a • b).
 */
inline double dot(const Vec a, const Vec b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * @brief Computes the cross product of two vectors.
 * @param a First vector.
 * @param b Second vector.
 * @return Cross product vector.
 */
inline Vec cross(Vec a, Vec b) 
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

/**
 * @brief Computes the length (magnitude) of a vector.
 * @param v Input vector.
 * @return The Euclidean length of the vector.
 */
inline double length(Vec v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief Compares two double values for approximate equality within a tolerance.
 *
 * This function checks if two floating-point numbers `a` and `b` are considered equal
 * by determining whether the absolute difference between them is less than a specified
 * epsilon tolerance. This is useful to handle precision errors inherent in floating-point arithmetic.
 *
 * @param a The first double value to compare.
 * @param b The second double value to compare.
 * @return true if the absolute difference between `a` and `b` is less than `eps`, false otherwise.
 */
inline bool equal(double a, double b)
{
    return std::abs(a - b) < eps;
}

/**
 * @brief Normalizes a vector to unit length.
 * @param v Input vector.
 * @return Normalized vector. Returns {0,0,0} if input vector length is zero.
 */
inline Vec normalize(Vec v) 
{
    const double len = length(v);
    if (equal(len, 0.0)) 
        return { 0, 0, 0 };
    return { v.x / len, v.y / len, v.z / len };
}

/**
 * @brief Calculates the normal vector of a triangle defined by three points.
 * @param a First vertex of the triangle.
 * @param b Second vertex of the triangle.
 * @param c Third vertex of the triangle.
 * @return Normalized normal vector of the triangle.
 */
inline Vec calculateNormal(Vec a, Vec b, Vec c) 
{
    return normalize(cross(b - a, c - a));
}

/**
 * @brief Parses a comma-separated string into a 3D vector.
 *
 * This function takes a string containing three comma-separated numeric values
 * representing the X, Y, and Z coordinates of a point (e.g., "1.0,2.5,3.2") and
 * converts them into a corresponding Vec object. Missing components default to 0.0.
 *
 * Example usage:
 * @code
 * Vec origin = parseOrigin("10.0,20.5,30.2");
 * // origin.x == 10.0, origin.y == 20.5, origin.z == 30.2
 * @endcode
 *
 * @param str The input string containing comma-separated numeric coordinates.
 * @return A Vec object representing the parsed 3D point.
 */
Vec parseOrigin(const std::string& str);
