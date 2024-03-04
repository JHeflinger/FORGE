#pragma once
#include "glm/glm.hpp"

class MathUtils {
public:
    static glm::mat4 ComposeTransform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
};
