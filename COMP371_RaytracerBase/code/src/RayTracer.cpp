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

RayTracer::RayTracer(const nlohmann::json &j)
{
    // Store all outputs
    for (const auto &output : j["output"])
    {
        outputs.push_back(output);
    }
    parseGeometry(j["geometry"]);
}

// Parses the geometry from the JSON and populates the objects vector
void RayTracer::parseGeometry(const nlohmann::json &geometryJson)
{
    for (auto &geometry : geometryJson)
    {
        if (geometry["type"] == "sphere")
        {
            Eigen::Vector3f centre(
                geometry["centre"][0],
                geometry["centre"][1],
                geometry["centre"][2]);

            float radius = geometry["radius"];

            objects.push_back(
                std::make_unique<Sphere>(centre, radius));
        }
        else if (geometry["type"] == "rectangle")
        {
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
            if (normal.dot(Eigen::Vector3f(0, 0, 1)) < 0)
            {
                // Flip winding
                objects.push_back(std::make_unique<Triangle>(v[0], v[2], v[1]));
                objects.push_back(std::make_unique<Triangle>(v[0], v[3], v[2]));
            }
            else
            {
                objects.push_back(std::make_unique<Triangle>(v[0], v[1], v[2]));
                objects.push_back(std::make_unique<Triangle>(v[0], v[2], v[3]));
            }
        }
    }
}

void RayTracer::run()
{
    // For each output configuration, render the scene and save the image
    for (const auto &output : outputs)
    {
        // Set image dimensions
        width = output["size"][0];
        height = output["size"][1];

        // Set background color
        backgroundColor = Eigen::Vector3f(output["bkc"][0], output["bkc"][1], output["bkc"][2]);

        // Set hit color to white if background is black, otherwise set it to black
        if (backgroundColor == Eigen::Vector3f(0.0f, 0.0f, 0.0f))
        {
            hitColor = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
        }
        else
        {
            hitColor = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
        }

        // Initialize camera
        Camera camera(
            Eigen::Vector3f(output["centre"][0], output["centre"][1], output["centre"][2]),
            Eigen::Vector3f(output["lookat"][0], output["lookat"][1], output["lookat"][2]),
            Eigen::Vector3f(output["up"][0], output["up"][1], output["up"][2]),
            output["fov"],
            width,
            height);

        // Loop over each pixel in the image
        size_t pixelCount = static_cast<size_t>(width) * height * 3;
        std::vector<double> buffer(pixelCount);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // For each pixel
                Ray ray = camera.generateRay(x, y);
                HitInfo closestHit;
                closestHit.t = std::numeric_limits<float>::infinity();
                bool hitAnything = false;

                for (const auto &object : objects)
                {
                    HitInfo hit;
                    if (object->intersect(ray, hit))
                    {
                        if (hit.t < closestHit.t)
                        {
                            closestHit = hit;   // update closest hit
                            hitAnything = true; // mark that we hit something
                        }
                    }
                }

                Eigen::Vector3f color;

                if (hitAnything)
                {
                    color = hitColor;
                }
                else
                {
                    color = backgroundColor; // Background color for no hit
                }

                // Write color to buffer 
                size_t idx = 3ull * (static_cast<size_t>(y) * width + x); // Calculate the index for the current pixel
                buffer[idx + 0] = color.x(); // Red channel
                buffer[idx + static_cast<size_t>(1)] = color.y(); // Green channel
                buffer[idx + static_cast<size_t>(2)] = color.z(); // Blue channel
            }
        }
        save_ppm(output["filename"], buffer, width, height);
    }
}