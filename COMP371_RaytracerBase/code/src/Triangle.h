#pragma once
#include <Eigen/Dense>
#include "Geometry.h"
#include "Ray.h"

struct HitInfo;

class Triangle : public Geometry
{
public:
    Triangle(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2,
            const Eigen::Vector3f& p3);
    virtual bool intersect(const Ray& ray, HitInfo& hit) const override;
private:
    const Eigen::Vector3f p1, p2, p3;
};