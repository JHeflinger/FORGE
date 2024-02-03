#pragma once
#include "glm/glm.hpp"

struct Viewport {
	uint32_t x;
	uint32_t y;
	uint32_t Width;
	uint32_t Height;
};

typedef glm::vec4 Color;

class Renderer {
public:
	static void Initialize();
	static void SetViewport(const Viewport& viewport);
	static void SetClearColor(const Color& color);
	static void Clear();
	static void Shutdown();
	//static void BeginScene(const Camera& camera);
	//static void EndScene();
	//static void Flush();
	//static void DrawLine(const Line& line);
	//static Statistics Stats();
	//static void ResetStats();
};
