#include "Forge.h"

Forge::Forge() {
    m_Panels = {};
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
