#include "Input.h"
#include "KeyCodes.h"
#include "MouseCodes.h"
#include <GLFW/glfw3.h>

static Ref<Window> s_Window;

bool Input::Initialize(Application* app) {
    if (app) {
        s_Window = app->GetWindow();
        if (s_Window) return true;
    }
    return false;
}

bool Input::IsKeyPressed(KeyCode key) {
	auto window = static_cast<GLFWwindow*>(s_Window->GetNativeWindow());
	auto state = glfwGetKey(window, static_cast<int32_t>(key));
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(MouseCode button) {
	auto window = static_cast<GLFWwindow*>(s_Window->GetNativeWindow());
	auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
	return state == GLFW_PRESS;
}

float Input::GetMouseX() {
	return  GetMousePosition().x;
}

float Input::GetMouseY() {
	return  GetMousePosition().y;
}

glm::vec2 Input::GetMousePosition() {
	auto window = static_cast<GLFWwindow*>(s_Window->GetNativeWindow());
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	return { (float)xpos, (float)ypos };
}