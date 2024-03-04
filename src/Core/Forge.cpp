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
	m_Camera.OnUpdate(ts);
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

	Circle circle;
	circle.Scale = {10.0f, 10.0f, 1.0f};
	circle.Thickness = 0.5f;
	circle.Fade = 0.5f;
	circle.CircleColor = {0.9f, 0.3f, 0.2f, 1.0f};
	Renderer::DrawCircle(circle);

	Renderer::EndScene();
}

void Forge::InitializePanels() {
    for (auto panel : m_Panels) {
        panel->Initialize();
    }
}

void Forge::UpdatePanels() {
    for (auto panel : m_Panels) {
        panel->Update(this);
    }
}
