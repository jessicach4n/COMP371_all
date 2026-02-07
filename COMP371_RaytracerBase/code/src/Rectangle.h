#pragma once
#include <Eigen/Dense>
#include "Geometry.h"
#include "Ray.h"

struct HitInfo;

class Rectangle : public Geometry
{
public:
    Rectangle(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2,
                const Eigen::Vector3f& v3, const Eigen::Vector3f& v4);
    virtual bool intersect(const Ray& ray, HitInfo& hit) const override;
private:
    const Eigen::Vector3f p1, p2, p3, p4;
};