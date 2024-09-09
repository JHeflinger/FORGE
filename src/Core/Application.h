#pragma once
#include <vector>
#include "Core/Editor.h"
#include "Core/Safety.h"
#include "Core/Window.h"
#include "Core/GUI.h"
#include "Core/Timestep.h"
#include "Events/ApplicationEvent.h"

class Application {
public:
	Application();
	bool Initialize();
	void Run();
	void Update(Timestep ts);
	void Shutdown();
	void OnEvent(Event& e);
public:
	Ref<Window> GetWindow() { return m_Window; }
private:
	bool OnWindowClosed(WindowCloseEvent& e);
	bool OnWindowResize(WindowResizeEvent& e);
private:
	Ref<Editor> m_Editor;
	Ref<Window> m_Window;
	Ref<GUI> m_GUI;
	bool m_Minimized = false;
	bool m_Running = true;
	float m_FrameTime = 0.0f;
};
