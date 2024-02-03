#include "Renderer.h"
#include <glad/glad.h>
#include "../Core/Log.h"

void Renderer::Initialize() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
}

void Renderer::SetViewport(const Viewport& viewport) {
	glViewport(viewport.x, viewport.y, viewport.Width, viewport.Height);
}

void Renderer::SetClearColor(const Color& color) {
	glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::Clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Shutdown() {
	
}
