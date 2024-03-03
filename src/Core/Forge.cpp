#include "Forge.h"
#include "../Renderer/Renderer.h"
#include "../Panels/ViewportPanel.h"
#include "../Panels/DeveloperPanel.h"

Forge::Forge() {
    m_Panels = { CreateRef<ViewportPanel>(), CreateRef<DeveloperPanel>() };
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

void Forge::OnEvent(Event& e) {
    m_Camera.OnEvent(e);
}

void Forge::Render() {
	Renderer::SetClearColor({0.1f, 0.5f, 0.3f, 1.0f});
	Renderer::Clear();
	Renderer::BeginScene(m_Camera);

	Renderer::DrawLine({{-5.0f, -5.0f, -5.0f}, {-5.0f, 5.0f, -5.0f}});
	Renderer::DrawLine({{-5.0f, 5.0f, -5.0f}, {5.0f, 5.0f, -5.0f}});
	Renderer::DrawLine({{5.0f, 5.0f, -5.0f}, {5.0f, -5.0f, -5.0f}});
	Renderer::DrawLine({{5.0f, -5.0f, -5.0f}, {-5.0f, -5.0f, -5.0f}});

	Renderer::DrawLine({{-5.0f, -5.0f, 5.0f}, {-5.0f, 5.0f, 5.0f}});
	Renderer::DrawLine({{-5.0f, 5.0f, 5.0f}, {5.0f, 5.0f, 5.0f}});
	Renderer::DrawLine({{5.0f, 5.0f, 5.0f}, {5.0f, -5.0f, 5.0f}});
	Renderer::DrawLine({{5.0f, -5.0f, 5.0f}, {-5.0f, -5.0f, 5.0f}});
    
	Renderer::DrawLine({{-5.0f, -5.0f, -5.0f}, {-5.0f, -5.0f, 5.0f}});
	Renderer::DrawLine({{-5.0f, 5.0f, -5.0f}, {-5.0f, 5.0f, 5.0f}});
	Renderer::DrawLine({{5.0f, 5.0f, -5.0f}, {5.0f, 5.0f, 5.0f}});
	Renderer::DrawLine({{5.0f, -5.0f, -5.0f}, {5.0f, -5.0f, 5.0f}});

	Renderer::EndScene();
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
        panel->Update(this);
    }
}
