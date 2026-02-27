#include "Triangle.h"
#include "Geometry.h"
#include "Ray.h"

Triangle::Triangle(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3)
    : p1(p1), p2(p2), p3(p3) {}

bool Triangle::intersect(const Ray& ray, HitInfo& hit) const {

    Eigen::Vector3f edge1 = p2 - p1;
    Eigen::Vector3f edge2 = p3 - p1;

    Eigen::Vector3f h = ray.getDirection().cross(edge2);
    float a = edge1.dot(h);

    if (std::abs(a) < 1e-8f)
        return false;

    float f = 1.0f / a;
    Eigen::Vector3f s = ray.getOrigin() - p1;
    float u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f)
        return false;

    Eigen::Vector3f q = s.cross(edge1);
    float v = f * ray.getDirection().dot(q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * edge2.dot(q);

    if (t > 1e-8f) { // ray intersection
        hit.t = t;
        hit.position = ray.getOrigin() + t * ray.getDirection();
        hit.normal = edge1.cross(edge2).normalized();
        hit.geometry = this;
        return true;
    }

    return false;
}