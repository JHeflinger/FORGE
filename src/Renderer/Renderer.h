#pragma once
#include "Camera.h"
#include "Buffer.h"
#include "VertexArray.h"
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

struct LineProperties {
	float LineWidth = 1.0f;
	Color LineColor = {1.0f, 1.0f, 1.0f, 1.0f};
};

struct RendererStatistics {
	uint32_t LineCount = 0;
};

struct LineVertex {
	glm::vec3 Position;
	Color LineColor;
};

struct RendererData {
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
	static void Flush();
	static void DrawLine(const Line& line);
	static RendererStatistics Stats();
	static void ResetStats();
public:
	static LineProperties GetLineProperties();
	static void SetLineProperties(const LineProperties& properties);
};
