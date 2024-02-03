#include "Renderer.h"
#include <GLFW/glfw3.h>
#include "../Core/Log.h"

void Renderer::Initialize() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
}
