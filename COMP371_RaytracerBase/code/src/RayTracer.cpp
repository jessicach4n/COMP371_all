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
#include "PointLight.h"
#include "AreaLight.h"
#include "../external/simpleppm.h"

RayTracer::RayTracer(const nlohmann::json &j)
{
    // Store all outputs
    for (const auto &output : j["output"])
    {
        outputs.push_back(output);
    }
    parseGeometry(j["geometry"]);
    parseLights(j["light"]);
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

            auto sphere = std::make_unique<Sphere>(centre, radius);

             // set material properties
             // coefficients
            sphere->material.ka = geometry["ka"]; // How much ambient light the surface absorbs
            sphere->material.kd = geometry["kd"]; // How much it scatters light in all directions
            sphere->material.ks = geometry["ks"]; // How much it produces shiny highlights
            sphere->material.pc = geometry["pc"]; // Phong exponent for shininess
            // colors
            sphere->material.ac = Eigen::Vector3f(geometry["ac"][0], geometry["ac"][1], geometry["ac"][2]); 
            sphere->material.dc = Eigen::Vector3f(geometry["dc"][0], geometry["dc"][1], geometry["dc"][2]); 
            sphere->material.sc = Eigen::Vector3f(geometry["sc"][0], geometry["sc"][1], geometry["sc"][2]); 

            objects.push_back(std::move(sphere));
        }
        else if (geometry["type"] == "rectangle")
        {
            // Load vertices
            Eigen::Vector3f v[4];
            v[0] = Eigen::Vector3f(geometry["p1"][0], geometry["p1"][1], geometry["p1"][2]);
            v[1] = Eigen::Vector3f(geometry["p2"][0], geometry["p2"][1], geometry["p2"][2]);
            v[2] = Eigen::Vector3f(geometry["p3"][0], geometry["p3"][1], geometry["p3"][2]);
            v[3] = Eigen::Vector3f(geometry["p4"][0], geometry["p4"][1], geometry["p4"][2]);

            // Load material properties
            Material mat;
            mat.ka = geometry["ka"];
            mat.kd = geometry["kd"];
            mat.ks = geometry["ks"];
            mat.pc = geometry["pc"];
            mat.ac = Eigen::Vector3f(geometry["ac"][0], geometry["ac"][1], geometry["ac"][2]);
            mat.dc = Eigen::Vector3f(geometry["dc"][0], geometry["dc"][1], geometry["dc"][2]);
            mat.sc = Eigen::Vector3f(geometry["sc"][0], geometry["sc"][1], geometry["sc"][2]);

            auto t1 = std::make_unique<Triangle>(v[0], v[1], v[2]);
            auto t2 = std::make_unique<Triangle>(v[0], v[2], v[3]);
            t1->material = mat;
            t2->material = mat;

            objects.push_back(std::move(t1));
            objects.push_back(std::move(t2));
        }
    }
}

void RayTracer::parseLights(const nlohmann::json &lightsJson)
{
    for (const auto &light : lightsJson)
    {
        if (light["type"] == "point")
        {
            Eigen::Vector3f centre(
                light["centre"][0],
                light["centre"][1],
                light["centre"][2]);

            // Diffuse intensity
            Eigen::Vector3f id(
                light["id"][0],
                light["id"][1],
                light["id"][2]);

            // Specular intensity
            Eigen::Vector3f is(
                light["is"][0],
                light["is"][1],
                light["is"][2]);
            
            bool use = light.value("use", true);

            lights.push_back(std::make_unique<PointLight>(centre, id, is, use));
        }
        else if (light["type"] == "area")
        {
            // Area light source geometry (rectangle)
            Eigen::Vector3f p1(
                light["p1"][0], 
                light["p1"][1], 
                light["p1"][2]);

            Eigen::Vector3f p2(
                light["p2"][0], 
                light["p2"][1], 
                light["p2"][2]);

            Eigen::Vector3f p3(
                light["p3"][0], 
                light["p3"][1], 
                light["p3"][2]);

            Eigen::Vector3f p4(
                light["p4"][0], 
                light["p4"][1], 
                light["p4"][2]);
            
            // Diffuse intensity
            Eigen::Vector3f id(
                light["id"][0],
                light["id"][1],
                light["id"][2]);

            // Specular intensity
            Eigen::Vector3f is(
                light["is"][0],
                light["is"][1],
                light["is"][2]);

            int n = light.value("n", 1);
            bool usecenter = light.value("usecenter", false); 

            lights.push_back(std::make_unique<AreaLight>(p1, p2, p3, p4, id, is, usecenter, n));
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

        // Initialize camera
        Camera camera(
            Eigen::Vector3f(output["centre"][0], output["centre"][1], output["centre"][2]),
            Eigen::Vector3f(output["lookat"][0], output["lookat"][1], output["lookat"][2]),
            Eigen::Vector3f(output["up"][0], output["up"][1], output["up"][2]),
            output["fov"],
            width,
            height);

        // Check Two-side render flag
        bool twoSideRender = output.value("twosiderender", true); // default true

        // Loop over each pixel in the image
        size_t pixelCount = static_cast<size_t>(width) * height * 3; // Each pixel needs values for RGB
        std::vector<double> buffer(pixelCount);

        // Ambient intensity of scene
        ai = Eigen::Vector3f(output["ai"][0], output["ai"][1], output["ai"][2]);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                // For each pixel
                Ray ray = camera.generateRay(x, y);
                HitInfo closestHit;
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
                    color = computeShading(ray, closestHit, twoSideRender);                
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

Eigen::Vector3f RayTracer::computeShading(const Ray& ray, const HitInfo& hit, const bool twoSideRender)
{
    const Material& material = hit.geometry->material; // get material properties

    Eigen::Vector3f v = -ray.getDirection().normalized(); // direction from hit point back towards the camera

    Eigen::Vector3f n = hit.normal.normalized(); // Surface normal at hit point

    // If two side rendering is on and the normal is facing away from the viewer
    if (twoSideRender && n.dot(v) < 0)
    {
        n = -n; // Flip normal so back faces are lit 
    }

    Eigen::Vector3f color = material.ka * ai.cwiseProduct(material.ac); // Init color with ambient term

    // accumulate diffuse aand specular contributions from each light
    for (const auto& light : lights) 
    {
        if (!light->use) continue; // Skip disabled lights (use == false)

        // calculate dir and dist from hit point to light
        Eigen::Vector3f lightPos = light->getPosition();
        Eigen::Vector3f toLight = lightPos - hit.position;
        float distanceToLight = toLight.norm();
        Eigen::Vector3f l = toLight.normalized(); // unit vector toward light

        if (isInShadow(hit.position, *light)) continue; // skip light if point is in shadow
        
        // Diffuse term
        float nDotL = std::max(0.0f, n.dot(l));
        Eigen::Vector3f diffuse = material.kd * nDotL * light->id.cwiseProduct(material.dc);

        // Specular term (Blinn-Phong)
        // H = halfway vector between thee light direction and view vector
        Eigen::Vector3f h = (l + v).normalized();
        float nDotH = std::max(0.0f, n.dot(h));
        Eigen::Vector3f specular = material.ks * std::pow(nDotH, material.pc) * light->is.cwiseProduct(material.sc);
        color += diffuse + specular;
    }

    return color.cwiseMin(Eigen::Vector3f(1.0f, 1.0f, 1.0f)); // Clamp color to [0, 1]
}

bool RayTracer::isInShadow(const Eigen::Vector3f& point, const Light& lightPos)
{
    // Calculate direction and distance from hit point to the light
    float distanceToLight = lightPos.getDistance(point);
    Eigen::Vector3f shadowRayDir = lightPos.getDirection(point);

    // Offset to prevent self-intersection
    Ray shadowRay(point + shadowRayDir * 0.1f, shadowRayDir); 

    // Test each objects in scene for intersection with shadow ray
    HitInfo shadowHit;
    for (const auto& object : objects)
    {
        if(!object->visible) continue; // Skip invisible objects
        if (object->intersect(shadowRay, shadowHit))
        {
            if (shadowHit.t < distanceToLight) // Shadow ray hit an object on the way to the light
            {
                return true; // In shadow
            }
        }
    }
    
    // Not in shadow
    return false; 
}