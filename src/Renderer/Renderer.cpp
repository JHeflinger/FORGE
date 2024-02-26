#include "Renderer.h"
#include <glad/glad.h>
#include "../Core/Log.h"

static RendererData s_Data;

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

void Renderer::BeginScene(const Camera& camera) {
	
}

void Renderer::EndScene() {

}

void Renderer::Flush() {

}

void Renderer::DrawLine(const Line& line) {

}

RendererStatistics Renderer::Stats() {
	return s_Data.Statistics;
}

void Renderer::ResetStats() {
	s_Data.Statistics = {};
}

LineProperties Renderer::GetLineProperties() {
	return s_Data.LineProps;
}

void Renderer::SetLineProperties(const LineProperties& properties) {
	s_Data.LineProps = properties;
}
