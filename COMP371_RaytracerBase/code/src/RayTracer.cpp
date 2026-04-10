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
            bool use = light.value("use", true);

            lights.push_back(std::make_unique<AreaLight>(p1, p2, p3, p4, id, is, usecenter, n, use));
        }
    }
}

// Check if any area light is present and enabled (use == true)
bool RayTracer::hasAreaLight() const
{
    // go throught all the lights and check if any of them is an area light with usecenter == false and use == true
    for (const auto &light : lights)
    {
        const AreaLight *al = dynamic_cast<const AreaLight *>(light.get());
        if (al && !al->usecenter && light->use)
            return true;
    }
    return false;
}

// Trace a ray into the scene and compute the color at the intersection point
Eigen::Vector3f RayTracer::traceRay(
    const Ray &ray, bool twoSideRender)
{
    HitInfo closestHit;
    bool hitAnything = false;

    for (const auto &object : objects)
    {
        HitInfo hit;
        if (object->intersect(ray, hit) && hit.t < closestHit.t)
        {
            closestHit = hit;
            hitAnything = true;
        }
    }

    if (hitAnything) 
    {
        return computeShading(ray, closestHit, twoSideRender); 
    }
    
    return backgroundColor;
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

        // Set ambient intensity
        ai = Eigen::Vector3f(output["ai"][0], output["ai"][1], output["ai"][2]);

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
        // Check antialiasing flag
        bool antialiasing   = output.value("antialiasing", false); // default false

        // if area light present, ignore antialiasing
        bool useAA = antialiasing && !hasAreaLight();

        // Parse raysperpixel
        int rppA = 2, rppB = 2, rppC = 1;
        if (useAA && output.contains("raysperpixel"))
        {
            auto rpp = output["raysperpixel"];
            if (rpp.size() == 1)
            {
                rppA = 1;
                rppB = 1;
                rppC = rpp[0];
            }
            else if (rpp.size() == 2)
            {
                rppA = rpp[0];
                rppB = rpp[0];
                rppC = rpp[1];
            }
            else if (rpp.size() == 3)
            {
                rppA = rpp[0];
                rppB = rpp[1];
                rppC = rpp[2];
            }
        }

        size_t pixelCount = static_cast<size_t>(width) * height * 3;
        std::vector<double> buffer(pixelCount);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                Eigen::Vector3f color(0.0f, 0.0f, 0.0f);

                if (useAA) // If antialiasing is enabled, shoot multiple rays per pixel and average the results
                {
                    int totalSamples = rppA * rppB * rppC; // Total number of rays per pixel
                    for (int gi = 0; gi < rppA; gi++) // grid index for x
                    {
                        for (int gj = 0; gj < rppB; gj++) // grid index for y
                        {
                            for (int s = 0; s < rppC; s++) // sample index for multiple samples per grid cell
                            {
                                float ru = static_cast<float>(rand()) / RAND_MAX; // random offset for x within the grid cell
                                float rv = static_cast<float>(rand()) / RAND_MAX; // random offset for y within the grid cell
                                float offsetX = ((gi + ru) / rppA) - 0.5f;      // offset in range [-0.5, 0.5]
                                float offsetY = ((gj + rv) / rppB) - 0.5f;      // offset in range [-0.5, 0.5]
                                Ray ray = camera.generateRay(x + offsetX, y + offsetY); 
                                color += traceRay(ray, twoSideRender);
                            }
                        }
                    }
                    color /= static_cast<float>(totalSamples); // Average the color from all samples
                }
                else
                {
                    color = traceRay(camera.generateRay(x, y), twoSideRender); // No antialiasing, just one ray per pixel
                }

                size_t idx = 3ull * (static_cast<size_t>(y) * width + x);
                buffer[idx + 0] = color.x();
                buffer[idx + 1] = color.y();
                buffer[idx + 2] = color.z();
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

        const AreaLight *al = dynamic_cast<const AreaLight *>(light.get());

        if (al && !al->usecenter)
        {
            int gridN = al->n;
            Eigen::Vector3f gridColor(0.0f, 0.0f, 0.0f);
            for (int i = 0; i < gridN; i++)
            {
                for (int j = 0; j < gridN; j++)
                {
                    Eigen::Vector3f samplePoint = al->getPosition(i, j, gridN);
                    Eigen::Vector3f toLight = samplePoint - hit.position;
                    float distanceToLight = toLight.norm();
                    Eigen::Vector3f l = toLight / distanceToLight; // unit vector toward light with distance attenuation

                    Ray shadowRay(hit.position + l * 1e-4f, l); 
                    bool inShadow = false;
                    for (const auto& obj : objects) 
                    {
                        if (!obj->visible) continue;
                        if (obj.get() == hit.geometry) continue; // skip self
                        HitInfo shadowHit;
                        if (obj->intersect(shadowRay, shadowHit) && shadowHit.t > 1e-4f && shadowHit.t < distanceToLight) {
                            inShadow = true;
                            break;
                        }
                    }
                    if (inShadow) continue;

                    float nDotL = std::max(0.0f, n.dot(l));
                    Eigen::Vector3f diffuse = material.kd * nDotL * light->id.cwiseProduct(material.dc);
                    Eigen::Vector3f h = (l + v).normalized();
                    float nDotH = std::max(0.0f, n.dot(h));
                    Eigen::Vector3f specular = material.ks * std::pow(nDotH,
                        material.pc) * light->is.cwiseProduct(material.sc);
                    gridColor += diffuse + specular;
                }
            }
            color += gridColor / static_cast<float>(gridN * gridN); 

        }
        else 
        {
            Eigen::Vector3f toLight = light->getPosition() - hit.position;
            float dist = toLight.norm();
            Eigen::Vector3f l = toLight / dist; // unit vector toward light with

            Ray shadowRay(hit.position + l * 1e-4f, l);
            bool blocked = false;
            for (const auto &obj : objects) {
                if (!obj->visible) continue;
                if (obj.get() == hit.geometry) continue; // skip self
                HitInfo sh;
                if (obj->intersect(shadowRay, sh) && sh.t > 1e-4f && sh.t < dist)
                { 
                    blocked = true; 
                    break; 
                }
            }
            if (blocked) continue;

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
    }
    return color.cwiseMin(Eigen::Vector3f(1.0f, 1.0f, 1.0f)); // Clamp color to [0, 1]
}