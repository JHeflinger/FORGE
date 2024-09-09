#pragma once
#include "Renderer/Camera.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "glm/glm.hpp"
#include <utility>

struct Viewport {
	uint32_t x;
	uint32_t y;
	uint32_t Width;
	uint32_t Height;
};

typedef glm::vec4 Color;
typedef std::pair<glm::vec3, glm::vec3> Line;

struct Circle {
	glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };
	Color CircleColor = {1.0f, 1.0f, 1.0f, 1.0f};
	float Thickness = 1.0f;
	float Fade = 0.005f;
};

struct Sphere {
	glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
	float Radius = 0.5f;
};

struct LineProperties {
	float LineWidth = 1.0f;
	Color LineColor = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct RendererStatistics {
	uint32_t LineCount = 0;
	uint32_t CircleCount = 0;
	uint32_t SphereCount = 0;
	uint32_t DrawCalls = 0;
};

struct LineVertex {
	glm::vec3 Position;
	Color LineColor;
};

struct CircleVertex {
	glm::vec3 WorldPosition;
	glm::vec3 LocalPosition;
	glm::vec4 Color;
	float Thickness;
	float Fade;
};

struct SphereVertex {
	glm::vec3 Position;
	float Radius;
};

struct RendererData {
	// Maximum constraints (change if needed)
	static const uint32_t MaxQuads = 20000;
	static const uint32_t MaxVertices = MaxQuads * 4;
	static const uint32_t MaxIndices = MaxQuads * 6;
	static const uint32_t MaxTextures = 32;

	// Quad stuff
	glm::vec4 QuadVertexPositions[4];

	// Shaders
	ShaderLibrary Shaders;

	// Line resources
	Ref<VertexArray> LineVertexArray;
	Ref<VertexBuffer> LineVertexBuffer;
	uint32_t LineVertexCount = 0;
	LineVertex* LineVertexBufferBase = nullptr;
	LineVertex* LineVertexBufferPtr = nullptr;

	// Circle resources
	Ref<VertexArray> CircleVertexArray;
	Ref<VertexBuffer> CircleVertexBuffer;
	uint32_t CircleIndexCount = 0;
	CircleVertex* CircleVertexBufferBase = nullptr;
	CircleVertex* CircleVertexBufferPtr = nullptr;

	// Sphere resources
	Ref<VertexArray> SphereVertexArray;
	Ref<VertexBuffer> SphereVertexBuffer;
	uint32_t SphereVertexCount = 0;
	SphereVertex* SphereVertexBufferBase = nullptr;
	SphereVertex* SphereVertexBufferPtr = nullptr;

	// Statistics and property tracking
	LineProperties LineProps;
	RendererStatistics Statistics;
};

class Renderer {
public:
	static void Initialize();
	static void SetViewport(const Viewport& viewport);
	static void SetClearColor(const Color& color);
	static void Clear();
	static void Shutdown();
	static void BeginScene(const Camera& camera);
	static void EndScene();
	static void StartBatch();
	static void NextBatch();
	static void Flush();
	static void DrawLine(const Line& line);
	static void DrawCircle(const Circle& circle);
	static void DrawSphere(const Sphere& sphere);
	static RendererStatistics Stats();
	static void ResetStats();
public:
	static LineProperties GetLineProperties();
	static void SetLineProperties(const LineProperties& properties);
private:
	static void SubmitLineProperties();
	static void SubmitLines();
	static void SubmitCircles();
	static void SubmitSpheres();
};
