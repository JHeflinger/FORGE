#pragma once
#include <vector>
#include "Structures.h"
#include "Safety.h"
#include "Timestep.h"
#include "../Renderer/Camera.h"
#include "../Panels/Panel.h"
#include "../Events/Event.h"

class Forge {
public:
    Forge();
    bool Initialize();
    void Update(Timestep ts);
    void Shutdown();
	void OnEvent(Event& e);
    void Render();
private:
    void InitializePanels();
    void UpdatePanels();
private:
    std::vector<Ref<Panel>> m_Panels;
    Camera m_Camera;
};
