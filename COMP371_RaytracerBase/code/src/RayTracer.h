#pragma once
#include "../external/json.hpp"
#include <memory>
#include <vector>

class Geometry;

class RayTracer {
public:
    RayTracer(const nlohmann::json& j);
    void run();

private:
    void parseOutput(const nlohmann::json& outputJson);
    void parseGeometry(const nlohmann::json& geometryJson);

    int width;
    int height;
    std::string outputFile;

    std::vector<std::unique_ptr<Geometry>> objects;
};