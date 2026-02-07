#include "Camera.h"

Camera::Camera(const Eigen::Vector3f &centre, const Eigen::Vector3f &lookat, const Eigen::Vector3f &up, float f, int w, int h)
    : origin(centre), forward(lookat), right(up), fov(f), width(w), height(h) {};

Ray Camera::generateRay(int x, int y) const {
    float u = (x + 0.5f) / width;
    float v = (y + 0.5f) / height;

    float px = (2.0f * u - 1.0f) * aspectRatio * tan(fov / 2.0f);
    float py = (1.0f - 2.0f * v) * tan(fov / 2.0f);

    Eigen::Vector3f dir = forward + px * right + py * cameraUp;
    dir.normalize();

    return Ray(origin, dir);
}