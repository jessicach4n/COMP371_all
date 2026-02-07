#include "Rectangle.h"
#include "Geometry.h"
#include "Ray.h"

Rectangle::Rectangle(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3, const Eigen::Vector3f& v4)
    : p1(v1), p2(v2), p3(v3), p4(v4) {}

bool Rectangle::intersect(const Ray& ray, HitInfo& hit) const {
    Eigen::Vector3f normal = (p2 - p1).cross(p3 - p1).normalized();
    
    float denom = normal.dot(ray.getDirection());
    if (fabs(denom) < 1e-6f) return false; // ray parallel to plane

    float t = normal.dot(p1 - ray.getOrigin()) / denom;
    if (t < 0 || t >= hit.t) return false; // intersection behind ray or farther than current hit

    Eigen::Vector3f P = ray.getOrigin() + t * ray.getDirection();

    auto pointInTriangle = [](const Eigen::Vector3f& p, const Eigen::Vector3f& a,
                              const Eigen::Vector3f& b, const Eigen::Vector3f& c) {
        Eigen::Vector3f v0 = c - a;
        Eigen::Vector3f v1 = b - a;
        Eigen::Vector3f v2 = p - a;

        float dot00 = v0.dot(v0);
        float dot01 = v0.dot(v1);
        float dot02 = v0.dot(v2);
        float dot11 = v1.dot(v1);
        float dot12 = v1.dot(v2);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        return (u >= 0) && (v >= 0) && (u + v <= 1);
    };

    if (pointInTriangle(P, p1, p2, p3) || pointInTriangle(P, p1, p3, p4)) {
        hit.position = P;
        hit.normal = normal;
        hit.t = t;
        return true;
    }

    return false;
}