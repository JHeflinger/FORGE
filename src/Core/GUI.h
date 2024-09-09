#pragma once
#include "Core/Window.h"
#include "Core/Safety.h"

class GUI {
public:
	GUI(Ref<Window> window);
	~GUI();
	void Initialize();
	void Shutdown();
	void OnEvent(Event& e);
	void Begin();
	void End();
	void SetTheme();
private:
	Ref<Window> m_Window;
};
