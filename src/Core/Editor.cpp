#include "Editor.h"
#include "../Renderer/Renderer.h"
#include "../Panels/ViewportPanel.h"
#include "../Panels/OverviewPanel.h"
#include "imgui.h"

Editor::Editor() {
    m_Panels = { CreateRef<ViewportPanel>(), CreateRef<OverviewPanel>() };
}

bool Editor::Initialize() {
	Renderer::Initialize();
	InitializePanels();
	return true;
}

void Editor::Update(Timestep ts) {
	DrawMenuBar();
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

	for (int i = 0; i < 90; i++) {
		Circle circle;
		circle.Scale = {(10.0f/90.0f) * i, (10.0f/90.0f) * i, 1.0f};
		//circle.Thickness = 0.5f;
		//circle.Fade = 0.5f;
		circle.CircleColor = {0.4, 0.3f, 0.7f, 1.0f};
		circle.Rotation = {glm::radians((float)i), glm::radians((float)i), 0.0f};
		Renderer::DrawCircle(circle);
	}

	Renderer::EndScene();
}

void Editor::InitializePanels() {
    for (auto panel : m_Panels) {
        panel->Initialize();
    }
}

void Editor::UpdatePanels() {
    for (auto panel : m_Panels) {
        panel->CallUpdate(this);
    }
}

void Editor::DrawMenuBar() {
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Project")) {
			if (ImGui::MenuItem("New"))
				FATAL("New Project not implemented!");
			if (ImGui::MenuItem("Open"))
				FATAL("Open Project not implemented!");
			if (ImGui::MenuItem("Export"))
				FATAL("Export not implemented!");
			if (ImGui::MenuItem("Import"))
				FATAL("Import not implemented!");
			if (ImGui::MenuItem("Settings"))
				FATAL("Settings not implemented!");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Panels")) {
			for (auto panel : m_Panels) {
				ImGui::MenuItem(panel->Name().c_str(), NULL, &(panel->m_Enabled));
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}