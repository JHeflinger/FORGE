#include "Forge.h"
#include "../Renderer/Renderer.h"
#include "../Panels/ViewportPanel.h"

Forge::Forge() {
    m_Panels = { CreateRef<ViewportPanel>() };
}

bool Forge::Initialize() {
	Renderer::Initialize();
    InitializePanels();
    return true;
}

void Forge::Update(Timestep ts) {
    UpdatePanels();
}

void Forge::Shutdown() {

}

void Forge::InitializePanels() {
    // initialize panels
    for (auto panel : m_Panels) {
        panel->Initialize();
    }
}

void Forge::UpdatePanels() {
    // update panels
    for (auto panel : m_Panels) {
        panel->Update();
    }
}
