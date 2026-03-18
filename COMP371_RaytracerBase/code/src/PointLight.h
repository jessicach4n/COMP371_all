#pragma once
#include "Light.h"

class PointLight : public Light {
public:
    PointLight(const Eigen::Vector3f& centre, const Eigen::Vector3f& id,
               const Eigen::Vector3f& is, bool use)
        : Light(id, is, centre, use) {}

    Eigen::Vector3f getPosition() const override {
        return centre; // point light is just a point so position is its centre
    }
};