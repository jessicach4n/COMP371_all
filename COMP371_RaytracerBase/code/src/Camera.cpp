#define _USE_MATH_DEFINES
#include "Camera.h"
#include <cmath>


Camera::Camera(
    const Eigen::Vector3f &centre,
    const Eigen::Vector3f &lookat,
    const Eigen::Vector3f &up,
    float f,
    int w,
    int h
)
: origin(centre), fov(f), width(w), height(h)
{
    aspectRatio = float(width) / float(height);
    forward = lookat.normalized();
    right = forward.cross(up).normalized();
    cameraUp = right.cross(forward).normalized();
}


Ray Camera::generateRay(int x, int y) const {
    // Normalize pixel coordinates to [0, 1] 
    // Add 0.5f to center ray on the pixel
    float u = (x + 0.5f) / width;
    float v = (y + 0.5f) / height;

    // Convert to radians
    float fovRad = fov * M_PI / 180.0f;

    // scaled by aspect ratio so pixels are not stretched
    float px = (2.0f * u - 1.0f) * aspectRatio * tan(fovRad / 2.0f); //stretch  on wider screens
    float py = (1.0f - 2.0f * v) * tan(fovRad / 2.0f);

    Eigen::Vector3f dir = forward + px * right + py * cameraUp;
    dir.normalize();

    return Ray(origin, dir);
}