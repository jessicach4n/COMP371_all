#include "Camera.h"

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
    // lookat is a vectorv for the forward direction
    forward = lookat.normalized();
    right = forward.cross(up).normalized();
    cameraUp = right.cross(forward).normalized();
}


Ray Camera::generateRay(int x, int y) const {
    // Convert pixel coordinates to normalized device coordinates [0, 1]
    // Adding 0.5f centers the ray within the pixel
    float u = (x + 0.5f) / width;
    float v = (y + 0.5f) / height;

    // Convert normalized coordinates to camera space coordinates [-1, 1]
    // Scale by aspect ratio and field of view to get the projected coordinates
    float fovRad = fov * M_PI / 180.0f;
    float px = (2.0f * u - 1.0f) * aspectRatio * tan(fovRad / 2.0f);
    float py = (1.0f - 2.0f * v) * tan(fovRad / 2.0f);

    // Construct ray direction using camera basis vectors
    // Combine forward direction with horizontal (right) and vertical (cameraUp) offsets
    Eigen::Vector3f dir = forward + px * right + py * cameraUp;
    dir.normalize();

    // Return a ray originating from the camera with the calculated direction
    return Ray(origin, dir);
}