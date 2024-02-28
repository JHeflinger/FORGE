#pragma once
#include <vector>
#include "Forge.h"
#include "Safety.h"
#include "Window.h"
#include "GUI.h"
#include "Timestep.h"
#include "../Events/ApplicationEvent.h"

class Application {
public:
	Application();
	bool Initialize();
	void Run();
	void Update(Timestep ts);
	void Shutdown();
	void OnEvent(Event& e);
private:
	bool OnWindowClosed(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);
private:
	Ref<Forge> m_Forge;
	Ref<Window> m_Window;
	Ref<GUI> m_GUI;
	bool m_Minimized = false;
	bool m_Running = true;
	float m_FrameTime = 0.0f;
};
