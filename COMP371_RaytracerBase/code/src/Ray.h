#pragma once
#include <Eigen/Dense>

class Ray
{
private:
    Eigen::Vector3f origin;     // Origin point of the ray
    Eigen::Vector3f direction;  // Direction vector of the ray (should be normalized)
public:
    // Constructor that initializes the ray with an origin and a direction
    Ray(const Eigen::Vector3f &origin, const Eigen::Vector3f &direction);

    // Getters
    Eigen::Vector3f getOrigin() const;
    Eigen::Vector3f getDirection() const;

    // Compute point at parameter t along the ray
    // P(t) = origin + t * direction
    Eigen::Vector3f getPointAt(float t) const;
};