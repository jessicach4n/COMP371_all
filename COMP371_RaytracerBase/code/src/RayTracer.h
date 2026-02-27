#pragma once

#include <memory>
#include <vector>
#include <string>
#include <Eigen/Dense>

#include "../external/json.hpp"
#include "Geometry.h"
#include "Camera.h"
#include "Light.h"

struct HitInfo;

class RayTracer {
public:
    RayTracer(const nlohmann::json& j);
    void run();
    
private:
    Eigen::Vector3f computeShading(const Ray& ray, const HitInfo& hit);
    bool isInShadow(const Eigen::Vector3f& point, const Light& lightPos);

    void parseGeometry(const nlohmann::json& geometryJson);
	void parseLights(const nlohmann::json& lightsJson);

    int width;
    int height;
    Eigen::Vector3f hitColor;
    Eigen::Vector3f backgroundColor;
    Eigen::Vector3f ai; // Ambient intensity
    
    std::vector<std::unique_ptr<Geometry>> objects;
    std::vector<std::unique_ptr<Light>> lights;
    std::vector<nlohmann::json> outputs;
};