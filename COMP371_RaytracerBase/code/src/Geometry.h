#pragma once

#include <Eigen/Dense>
#include <limits>

#include "Ray.h"

class Geometry;

// surface appearance properties
struct Material {
    // colors
    Eigen::Vector3f ac = Eigen::Vector3f::Zero(); 
    Eigen::Vector3f dc = Eigen::Vector3f::Zero();
    Eigen::Vector3f sc = Eigen::Vector3f::Zero();

    // coefficients for strength of each light term
    float ka = 0.0f;
    float kd = 0.0f; 
    float ks = 0.0f; 
    float pc = 1.0f; // phong exponent
};

// stores info about ray-object intersection
struct HitInfo
{
    float t = std::numeric_limits<float>::infinity(); // distance along ray: O + t*d
    Eigen::Vector3f position = Eigen::Vector3f::Zero(); // world position of hit point
    Eigen::Vector3f normal = Eigen::Vector3f::Zero(); // surface normal at hit point
    const Geometry* geometry = nullptr; // ptr to hit object
    inline const Material& material() const;  // hit object's material 
};

class Geometry
{
public:
    virtual ~Geometry() = default;

    // Intersection test: returns true if hit, and fills HitInfo
    virtual bool intersect(const Ray& ray, HitInfo& hit) const = 0;

    bool visible = true; // If false, object is ignored in shadow ray tests

    Material material; // Material properties for shading
};

// get material of the hit object
inline const Material& HitInfo::material() const {
    return geometry->material;
}