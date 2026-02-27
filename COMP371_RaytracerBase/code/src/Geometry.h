#pragma once

#include <Eigen/Dense>
#include <limits>

#include "Ray.h"

class Geometry;

struct Material {
    Eigen::Vector3f ac = Eigen::Vector3f::Zero();
    Eigen::Vector3f dc = Eigen::Vector3f::Zero();
    Eigen::Vector3f sc = Eigen::Vector3f::Zero();
    float ka = 0.0f;
    float kd = 0.0f;
    float ks = 0.0f;
    float pc = 1.0f;
};

struct HitInfo
{
    float t = std::numeric_limits<float>::infinity();
    Eigen::Vector3f position = Eigen::Vector3f::Zero();
    Eigen::Vector3f normal = Eigen::Vector3f::Zero();
    const Geometry* geometry = nullptr;
    inline const Material& material() const;
};

class Geometry
{
public:
    virtual ~Geometry() = default;

    // Intersection test: returns true if hit, and fills HitInfo
    virtual bool intersect(const Ray& ray, HitInfo& hit) const = 0;

    bool visible = true;

    Material material; // Material properties for shading
};

inline const Material& HitInfo::material() const {
    return geometry->material;
}