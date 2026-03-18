#include "Triangle.h"
#include "Geometry.h"
#include "Ray.h"

Triangle::Triangle(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2, const Eigen::Vector3f& p3)
    : p1(p1), p2(p2), p3(p3) {}

bool Triangle::intersect(const Ray& ray, HitInfo& hit) const {
    // Two edges of the triangle from p1
    Eigen::Vector3f edge1 = p2 - p1;
    Eigen::Vector3f edge2 = p3 - p1;

    // h is used too calculate the determinant
    Eigen::Vector3f h = ray.getDirection().cross(edge2);
    float determinant = edge1.dot(h); 

    // if near zero, ray is parallel to triangle so no intersection
    if (std::abs(determinant) < 1e-8f)
        return false;

    float invDeterminant = 1.0f / determinant; // inverse determinant, multiplication cheaper than division
    Eigen::Vector3f s = ray.getOrigin() - p1; // vector from p1 to ray origin

    float u = invDeterminant * s.dot(h); // barycentric coordinate u
    if (u < 0.0f || u > 1.0f) 
        return false;

    // helper vector
    Eigen::Vector3f q = s.cross(edge1);

    float v = invDeterminant * ray.getDirection().dot(q); // barycentric coordinate v
    if (v < 0.0f || u + v > 1.0f) 
        return false;

    // t: distance along the ray to the intersection point
    float t = invDeterminant * edge2.dot(q);

    if (t > 1e-8f) { // ray intersection 
        hit.t = t;
        hit.position = ray.getOrigin() + t * ray.getDirection();
        hit.normal = edge1.cross(edge2).normalized();
        hit.geometry = this;
        return true;
    }

    return false;
}