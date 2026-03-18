#pragma once
#include "Light.h"

class AreaLight : public Light {
public:
    Eigen::Vector3f p1, p2, p3, p4;
    bool usecenter;
    int n;

    AreaLight(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2,
              const Eigen::Vector3f& p3, const Eigen::Vector3f& p4,
              const Eigen::Vector3f& id, const Eigen::Vector3f& is,
              bool usecenter, int n)
        : Light(id, is, (p1 + p2 + p3 + p4) / 4.0f, usecenter),
          p1(p1), p2(p2), p3(p3), p4(p4),
          usecenter(usecenter), n(n) {}

    Eigen::Vector3f getPosition() const override {
        if (usecenter) return centre; // treat as a point light
        else return centre; //  TODO: implement area light 
    }
};