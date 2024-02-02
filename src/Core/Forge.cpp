#include "Forge.h"
#include "../Panels/ViewportPanel.h"

Forge::Forge() {
    m_Panels = { CreateRef<ViewportPanel>() };
}

bool Forge::Initialize() {
    InitializePanels();
    return true;
}

void Forge::Run() {
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
