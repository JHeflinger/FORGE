#include "Renderer.h"
#include <glad/glad.h>
#include "../Core/Log.h"
#include "Static.h"

static RendererData s_Data;

void Renderer::Initialize() {
	// enable basic preset parameters
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);

	// internal shaders
	s_Data.Shaders.Add(CreateRef<Shader>(StaticShaders::LineGLSL(), "Line"));

	// lines 
	s_Data.LineVertexArray = CreateRef<VertexArray>();
	s_Data.LineVertexBuffer = CreateRef<VertexBuffer>(s_Data.MaxVertices * sizeof(LineVertex));
	s_Data.LineVertexBuffer->SetLayout({
		{ ShaderDataTypes::FLOAT3, "a_Position" },
		{ ShaderDataTypes::FLOAT4, "a_Color" }
	});
	s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
	s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];
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
	delete[] s_Data.LineVertexBufferBase;
}

void Renderer::BeginScene(const Camera& camera) {
	// set the view projection
	s_Data.Shaders.Get("Line")->Bind();
	s_Data.Shaders.Get("Line")->SetMat4("u_ViewProjection", camera.GetProjection());
	
	StartBatch();
}

void Renderer::EndScene() {
	Flush();
}

void Renderer::StartBatch() {
	s_Data.LineVertexCount = 0;
	s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
}

void Renderer::NextBatch() {
	Flush();
	StartBatch();
}

void Renderer::Flush() {
	bool drawcall = false;
	if (s_Data.LineVertexCount) {
		uint32_t datasize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
		s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, datasize);
		s_Data.Shaders.Get("Line")->Bind();
		SubmitLineProperties();
		SubmitLines();
		drawcall = true;
	}
	if (drawcall) s_Data.Statistics.DrawCalls++;
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

void Renderer::SubmitLineProperties() {

}

void Renderer::SubmitLines() {

}
