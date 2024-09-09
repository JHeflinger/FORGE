#pragma once
#include <string>
#include "Events/Event.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using EventCallbackFn = std::function<void(Event&)>;

struct WindowProperties {
	std::string Name;
	uint32_t Width;
	uint32_t Height;
	bool Fullscreen;
	WindowProperties(const std::string& name = "FORGE",
				  uint32_t width = 1600,
				  uint32_t height = 900,
				  bool fullscreen = true)
	:Name(name), Width(width), Height(height), Fullscreen(fullscreen) {}
};

struct WindowData {
	std::string Name;
	uint32_t Width;
	uint32_t Height;
	bool VSync;
	EventCallbackFn EventCallback;
	bool Fullscreen = true;
};

class Window {
public:
	Window(const WindowProperties& properties);
	~Window();
	void Update();
	uint32_t Width() const { return m_Data.Width; }
	uint32_t Height() const { return m_Data.Height; }
	void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
	void SetVSync(bool enabled);
	bool IsVSync() const { return m_Data.VSync; }
	void* GetNativeWindow() const { return m_Window; }
private:
	GLFWwindow* m_Window;
	WindowData m_Data;
};
