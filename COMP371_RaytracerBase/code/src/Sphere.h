#pragma once
#include <Eigen/Dense>
#include "Geometry.h"
#include "Ray.h"

struct HitInfo;

class Sphere : public Geometry
{
public:
    Sphere(const Eigen::Vector3f& c, float r);
    virtual bool intersect(const Ray& ray, HitInfo& hit) const override;
private:
    const Eigen::Vector3f centre;
    const float radius;
};