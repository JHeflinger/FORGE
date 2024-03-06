#include "Editor.h"
#include "../Renderer/Renderer.h"
#include "../Panels/ViewportPanel.h"
#include "../Panels/HierarchyPanel.h"

Editor::Editor() {
    m_Panels = { CreateRef<ViewportPanel>(), CreateRef<HierarchyPanel>() };
}

bool Editor::Initialize() {
	Renderer::Initialize();
	InitializePanels();
	return true;
}

void Editor::Update(Timestep ts) {
    UpdatePanels();
	m_Camera.OnUpdate(ts);
}

void Editor::Shutdown() {

}

void Editor::OnEvent(Event& e) {
    m_Camera.OnEvent(e);
}

void Editor::Render() {
	Renderer::SetClearColor({0.1f, 0.5f, 0.3f, 1.0f});
	Renderer::Clear();
	Renderer::BeginScene(m_Camera);

	LineProperties props = Renderer::GetLineProperties();
	props.LineColor = { 0.3f, 0.2f, 0.9f, 1.0f };
	props.LineWidth = 5.0f;
	Renderer::SetLineProperties(props);

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

void Editor::InitializePanels() {
    for (auto panel : m_Panels) {
        panel->Initialize();
    }
}

void Editor::UpdatePanels() {
    for (auto panel : m_Panels) {
        panel->Update(this);
    }
}
