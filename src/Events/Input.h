#pragma once
#include "Events/KeyCodes.h"
#include "Events/MouseCodes.h"
#include "Core/Application.h"
#include "glm/glm.hpp"

class Input {
public:
    static bool Initialize(Application* app);
	static bool IsKeyPressed(KeyCode key);
	static bool IsMouseButtonPressed(MouseCode button);
	static float GetMouseX();
	static float GetMouseY();
	static glm::vec2 GetMousePosition();
};
