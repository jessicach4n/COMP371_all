#include <iostream>
#include <string>
#include <limits>
#include <Eigen/Dense>

#include "RayTracer.h"
#include "Ray.h"
#include "Sphere.h"
#include "Geometry.h"
#include "Camera.h"
#include "Triangle.h"
#include "../external/simpleppm.h"

RayTracer::RayTracer(const nlohmann::json& j) 
    : width(j["output"][0]["size"][0]),
      height(j["output"][0]["size"][1]),
      outputFile(j["output"][0]["filename"]),
      backgroundColor(
          j["output"][0]["bkc"][0],
          j["output"][0]["bkc"][1],
          j["output"][0]["bkc"][2]
      ),
      camera(
          Eigen::Vector3f(j["output"][0]["centre"][0], j["output"][0]["centre"][1], j["output"][0]["centre"][2]),
          Eigen::Vector3f(j["output"][0]["lookat"][0], j["output"][0]["lookat"][1], j["output"][0]["lookat"][2]),
          Eigen::Vector3f(j["output"][0]["up"][0], j["output"][0]["up"][1], j["output"][0]["up"][2]),
          j["output"][0]["fov"],
          j["output"][0]["size"][0],
          j["output"][0]["size"][1]
      )
{
    parseGeometry(j["geometry"]);
}

void RayTracer::parseGeometry(const nlohmann::json& geometryJson) {
    for (auto& geometry : geometryJson) {
        if (geometry["type"] == "sphere") {
            Eigen::Vector3f centre (
                geometry["centre"][0],
                geometry["centre"][1],
                geometry["centre"][2]
            );

            float radius = geometry["radius"];

            objects.push_back(
                std::make_unique<Sphere>(centre, radius)
            );
        }
        else if (geometry["type"] == "rectangle") {
            // Load vertices
            Eigen::Vector3f v[4];
            v[0] = Eigen::Vector3f(geometry["p1"][0], geometry["p1"][1], geometry["p1"][2]);
            v[1] = Eigen::Vector3f(geometry["p2"][0], geometry["p2"][1], geometry["p2"][2]);
            v[2] = Eigen::Vector3f(geometry["p3"][0], geometry["p3"][1], geometry["p3"][2]);
            v[3] = Eigen::Vector3f(geometry["p4"][0], geometry["p4"][1], geometry["p4"][2]);

            // Compute two possible diagonals
            Eigen::Vector3f d1 = v[2] - v[0];
            Eigen::Vector3f d2 = v[3] - v[1];

            Eigen::Vector3f normal = (v[1] - v[0]).cross(v[2] - v[0]);
            if (normal.dot(Eigen::Vector3f(0,0,1)) < 0) { 
                // Flip winding
                objects.push_back(std::make_unique<Triangle>(v[0], v[2], v[1]));
                objects.push_back(std::make_unique<Triangle>(v[0], v[3], v[2]));
            } else {
                objects.push_back(std::make_unique<Triangle>(v[0], v[1], v[2]));
                objects.push_back(std::make_unique<Triangle>(v[0], v[2], v[3]));
            }

        }
    }
}

void RayTracer::run() {
    std::vector<double> buffer(width * height * 3);
        
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // For each pixel
            Ray ray = camera.generateRay(x, y); 
            HitInfo closestHit;  
            closestHit.t = std::numeric_limits<float>::infinity(); 
            bool hitAnything = false; 

            for (const auto& object : objects) {
                HitInfo hit;
                if (object->intersect(ray, hit)) {
                    if (hit.t < closestHit.t) {
                        closestHit = hit; // update closest hit
                        hitAnything = true; // mark that we hit something
                    }
                }
            }

            Eigen::Vector3f color;
            
            if (hitAnything) {
                color = Eigen::Vector3f(0.0f, 0.0f, 0.0f); // Black for hit
            }
            else {
                color = backgroundColor; // Background color for no hit
            }

            int idx = 3 * (y * width + x);
            buffer[idx + 0] = color.x();
            buffer[idx + 1] = color.y();
            buffer[idx + 2] = color.z();    
        }
    }
    save_ppm(outputFile, buffer, width, height);
}