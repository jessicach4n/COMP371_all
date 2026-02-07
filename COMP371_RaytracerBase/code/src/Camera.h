#pragma once
#include <Eigen/Dense>
#include "Ray.h"

class Camera
{
private:
    Eigen::Vector3f origin;
    Eigen::Vector3f forward;
    Eigen::Vector3f right;
    Eigen::Vector3f cameraUp;

    float fov;
    float aspectRatio;
    int width;
    int height;

public:
    Camera(
        const Eigen::Vector3f& centre,
        const Eigen::Vector3f& lookat,
        const Eigen::Vector3f& up,
        float f,
        int w,
        int h);

    Ray generateRay(int x, int y) const;
};
