#pragma once
#include <Eigen/Dense>
#include "Geometry.h"

class Sphere : public Geometry
{
public:
    Sphere(const Eigen::Vector3f& c, float r);
    virtual bool intersect(const Ray& ray, HitInfo& hit) const override;
private:
    Eigen::Vector3f centre;
    float radius;
};