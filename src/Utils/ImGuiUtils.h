#pragma once
#include "glm/glm.hpp"
#include <string>

class ImGuiUtils {
public:
	static bool DrawVec2Control(const std::string& label, glm::dvec2& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static bool DrawVec3Control(const std::string& label, glm::dvec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static bool DrawVec4Control(const std::string& label, glm::dvec4& values, float resetValue = 0.0f, float columnWidth = 100.0f);
};
