#pragma once

#include <Eigen/Dense>
#include <limits>

#include "Ray.h"

class Geometry;

struct HitInfo
{
    float t = std::numeric_limits<float>::infinity();
    Eigen::Vector3f position = Eigen::Vector3f::Zero();
    Eigen::Vector3f normal = Eigen::Vector3f::Zero();
    const Geometry* geometry = nullptr;
};

class Geometry
{
public:
    virtual ~Geometry() = default;

    // Intersection test: returns true if hit, and fills HitInfo
    virtual bool intersect(const Ray& ray, HitInfo& hit) const = 0;

    bool visible = true;
};