#pragma once
#include <Eigen/Dense>

class Light
{
public:
    Eigen::Vector3f id; // Diffuse intensity
    Eigen::Vector3f is; // Specular intensity
    Eigen::Vector3f centre;
    bool use;

    Light(const Eigen::Vector3f& id, const Eigen::Vector3f& is, const Eigen::Vector3f& centre, bool use)
        : id(id), is(is), centre(centre), use(use) {}

    virtual ~Light() = default;

    virtual Eigen::Vector3f getPosition() const = 0; // Pure virtual function to get the position of the light

    // unit vector from hit point toward the light
    virtual Eigen::Vector3f getDirection(const Eigen::Vector3f& point) const {
        return (getPosition() - point).normalized(); 
    }

    // distance from hit point to the light
    virtual float getDistance(const Eigen::Vector3f& point) const {
        return (getPosition() - point).norm();
    }
};