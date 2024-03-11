#pragma once
#include <vector>
#include "Safety.h"
#include "Timestep.h"
#include "Serializer.h"
#include "Simulation.h"
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
    std::vector<Ref<Panel>> GetPanels() { return m_Panels; }
    Ref<Simulation> GetSimulation() { return m_Simulation; }
    Ref<Camera> GetCamera() { return m_Camera; }
private:
    void InitializePanels();
    void UpdatePanels();
    void DrawMenuBar();
private:
    std::vector<Ref<Panel>> m_Panels;
    Ref<Simulation> m_Simulation;
    Ref<Camera> m_Camera;
};