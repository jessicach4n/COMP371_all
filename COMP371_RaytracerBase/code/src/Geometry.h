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

    // Material properties (defaults can be overridden by JSON parsing)
    float ka = 0.1f;
    float kd = 0.7f;
    float ks = 0.2f;
    float pc = 16.0f;

    // RGB colors
    Eigen::Vector3f ac = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    Eigen::Vector3f dc = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    Eigen::Vector3f sc = Eigen::Vector3f(1.0f, 1.0f, 1.0f);

    bool visible = true;
};