#include "Ray.h"

// Constructor that stores the ray with an origin and a direction
Ray::Ray(const Eigen::Vector3f& o, const Eigen::Vector3f& d)
    : origin(o), direction(d.normalized()) // Ensure direction is normalized
{}

// Getter for the origin of the ray
Eigen::Vector3f Ray::getOrigin() const
{
    return origin;
}

// Getter for the direction of the ray
Eigen::Vector3f Ray::getDirection() const
{
    return direction;
}

// Compute point at parameter t along the ray
Eigen::Vector3f Ray::getPointAt(float t) const
{
    return origin + t * direction;
}
