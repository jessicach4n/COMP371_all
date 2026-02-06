#include "RayTracer.h"
#include "Sphere.h"
#include <iostream>
#include <string>
#include <Eigen/Dense>

class Ray;

RayTracer::RayTracer(const nlohmann::json& j) {
    // Constructor implementation (parse JSON and initialize scene)
    parseOutput(j["output"][0]);
    parseGeometry(j["geometry"]);
}

void RayTracer::parseOutput(const nlohmann::json& outputJson) {
    width = outputJson["size"][0];
    height = outputJson["size"][0];
    outputFile = outputJson["filename"];
    // Camera centre
    // Lookat
    // Up
    // Fov
    // Background color

}

void RayTracer::parseGeometry(const nlohmann::json& geometryJson) {
    for (auto& geometry : geometryJson) {
        if (geometry["type"] == "sphere") {
            Eigen::Vector3f centreCoordinates(
                geometry["centre"][0],
                geometry["centre"][1],
                geometry["centre"][2]
            );

            float radius = geometry["radius"];

            objects.push_back(
                std::make_unique<Sphere>(centreCoordinates, radius)
            );

        }
    }
}

void RayTracer::run() {
    // Ray tracing implementation
}