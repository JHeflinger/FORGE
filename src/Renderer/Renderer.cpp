#include "Renderer.h"
#include <glad/glad.h>
#include "../Core/Log.h"
#include "../Utils/MathUtils.h"
#include "Static.h"

static RendererData s_Data;

void Renderer::Initialize() {
	// enable basic preset parameters
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);

	// quad stuff
	s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
	s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
	s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
	s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
	uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];
	uint32_t offset = 0;
	for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
		quadIndices[i + 0] = offset + 0;
		quadIndices[i + 1] = offset + 1;
		quadIndices[i + 2] = offset + 2;

		quadIndices[i + 3] = offset + 2;
		quadIndices[i + 4] = offset + 3;
		quadIndices[i + 5] = offset + 0;
		offset += 4;
	}
	Ref<IndexBuffer> quadIB = CreateRef<IndexBuffer>(quadIndices, s_Data.MaxIndices);
	delete[] quadIndices;

	// internal shaders
	s_Data.Shaders.Add(CreateRef<Shader>(StaticShaders::LineGLSL(), "Line"));
	s_Data.Shaders.Add(CreateRef<Shader>(StaticShaders::CircleGLSL(), "Circle"));

	// lines 
	s_Data.LineVertexArray = CreateRef<VertexArray>();
	s_Data.LineVertexBuffer = CreateRef<VertexBuffer>(s_Data.MaxVertices * sizeof(LineVertex));
	s_Data.LineVertexBuffer->SetLayout({
		{ ShaderDataTypes::FLOAT3, "a_Position" },
		{ ShaderDataTypes::FLOAT4, "a_Color" }
	});
	s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
	s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];

	// circles
	s_Data.CircleVertexArray = CreateRef<VertexArray>();
	s_Data.CircleVertexBuffer = CreateRef<VertexBuffer>(s_Data.MaxVertices * sizeof(CircleVertex));
	s_Data.CircleVertexBuffer->SetLayout({
			{ ShaderDataTypes::FLOAT3, "a_WorldPosition" },
			{ ShaderDataTypes::FLOAT3, "a_LocalPosition" },
			{ ShaderDataTypes::FLOAT4, "a_Color" },
			{ ShaderDataTypes::FLOAT, "a_Thickness" },
			{ ShaderDataTypes::FLOAT, "a_Fade" },
	});
	s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);
	s_Data.CircleVertexArray->SetIndexBuffer(quadIB); // note: using quad ib on purpose
	s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];
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
	delete[] s_Data.CircleVertexBufferBase;
}

void Renderer::BeginScene(const Camera& camera) {
	// set the view projection
	s_Data.Shaders.Get("Line")->Bind();
	s_Data.Shaders.Get("Line")->SetMat4("u_ViewProjection", camera.GetViewProjection());
	s_Data.Shaders.Get("Circle")->Bind();
	s_Data.Shaders.Get("Circle")->SetMat4("u_ViewProjection", camera.GetViewProjection());
	
	StartBatch();
}

void Renderer::EndScene() {
	Flush();
}

void Renderer::StartBatch() {
	// Prepare lines
	s_Data.LineVertexCount = 0;
	s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

	// Prepare circles
	s_Data.CircleIndexCount = 0;
	s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;
}

void Renderer::NextBatch() {
	Flush();
	StartBatch();
}

void Renderer::Flush() {
	bool drawcall = false;

	// draw circles
	if (s_Data.CircleIndexCount) {
		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
		s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);
		s_Data.Shaders.Get("Circle")->Bind();
		SubmitCircles();
		drawcall = true;
	}

	// draw lines
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
	s_Data.LineVertexBufferPtr->Position = line.first;
	s_Data.LineVertexBufferPtr->LineColor = s_Data.LineProps.LineColor;
	s_Data.LineVertexBufferPtr++;

	s_Data.LineVertexBufferPtr->Position = line.second;
	s_Data.LineVertexBufferPtr->LineColor = s_Data.LineProps.LineColor;
	s_Data.LineVertexBufferPtr++;

	s_Data.LineVertexCount += 2;
	s_Data.Statistics.LineCount++;
}

void Renderer::DrawCircle(const Circle& circle) {
	if (s_Data.CircleIndexCount >= s_Data.MaxIndices) NextBatch();

	glm::mat4 transform = MathUtils::ComposeTransform(circle.Position, circle.Rotation, circle.Scale);
	for (size_t i = 0; i < 4; i++) {
		s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
		s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
		s_Data.CircleVertexBufferPtr->Color = circle.CircleColor;
		s_Data.CircleVertexBufferPtr->Thickness = circle.Thickness;
		s_Data.CircleVertexBufferPtr->Fade = circle.Fade;
		s_Data.CircleVertexBufferPtr++;
	}

	s_Data.CircleIndexCount += 6;
	s_Data.Statistics.CircleCount++;
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
	glLineWidth(s_Data.LineProps.LineWidth);
}

void Renderer::SubmitLines() {
	s_Data.LineVertexArray->Bind();
	glDrawArrays(GL_LINES, 0, s_Data.LineVertexCount);
}

void Renderer::SubmitCircles() {
	s_Data.CircleVertexArray->Bind();
	uint32_t count = s_Data.CircleIndexCount ? s_Data.CircleIndexCount : s_Data.CircleVertexArray->GetIndexBuffer()->GetCount();
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}