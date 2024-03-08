#pragma once
#include <vector>
#include "Safety.h"
#include "Timestep.h"
#include "Serializer.h"
#include "../Renderer/Camera.h"
#include "../Panels/Panel.h"
#include "../Events/Event.h"

class Editor {
public:
    Editor();
    bool Initialize();
    void Update(Timestep ts);
    void Shutdown();
	void OnEvent(Event& e);
    void Render();
public:
    Camera& GetCamera() { return m_Camera; }
    std::vector<Ref<Panel>> GetPanels() { return m_Panels; }
private:
    void InitializePanels();
    void UpdatePanels();
    void DrawMenuBar();
private:
    std::vector<Ref<Panel>> m_Panels;
    Camera m_Camera;
};