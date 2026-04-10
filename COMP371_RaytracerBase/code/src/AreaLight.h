#pragma once
#include <cstdlib>
#include "Light.h"

class AreaLight : public Light {
public:
    Eigen::Vector3f p1, p2, p3, p4;
    bool usecenter;
    int n;

    AreaLight(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2,
              const Eigen::Vector3f& p3, const Eigen::Vector3f& p4,
              const Eigen::Vector3f& id, const Eigen::Vector3f& is,
              bool usecenter, int n, bool use = true)
        : Light(id, is, (p1 + p2 + p3 + p4) / 4.0f, use), 
          p1(p1), p2(p2), p3(p3), p4(p4),
          usecenter(usecenter), n(n) {}

    Eigen::Vector3f getPosition(int i, int j, int gridSize) const {
        if (usecenter) return centre; // treat as a point light
        
        // Stratified random offsets within the grid cell (i, j)
        float u = (i + 0.5f) / static_cast<float>(gridSize);
        float v = (j + 0.5f) / static_cast<float>(gridSize);

        // Bilinear interpolation
        return (1 - u) * (1 - v) * p1
             + u * (1 - v) * p2
             + u * v * p3
             + (1 - u) * v * p4;
    }

    Eigen::Vector3f getPosition() const override {
        return getPosition(0, 0, 1); 
    }
};