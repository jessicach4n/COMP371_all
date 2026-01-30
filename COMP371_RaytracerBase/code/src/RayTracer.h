#pragma once
#include "../external/json.hpp"

class RayTracer {
public:
    RayTracer(const nlohmann::json& j);
    void run();
};