#pragma once

#include <memory>
#include <vector>
#include <string>
#include <Eigen/Dense>

#include "../external/json.hpp"
#include "Geometry.h"
#include "Camera.h"

struct HitInfo;

class RayTracer {
public:
    RayTracer(const nlohmann::json& j);
    void run();

private:
    void parseGeometry(const nlohmann::json& geometryJson);

    int width;
    int height;
    std::string outputFile;
    Eigen::Vector3f backgroundColor;
    Eigen::Vector3f hitColor;
    Camera camera;

    std::vector<std::unique_ptr<Geometry>> objects;
};