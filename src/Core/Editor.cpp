#include "Editor.h"
#include "../Renderer/Renderer.h"
#include "../Panels/ViewportPanel.h"
#include "../Panels/OverviewPanel.h"
#include "../Panels/ResourcePanel.h"
#include "../Events/Input.h"
#include "imgui.h"

Editor::Editor() {
    m_Panels = { 
		CreateRef<ViewportPanel>(), 
		CreateRef<OverviewPanel>(),
		CreateRef<ResourcePanel>(),
	};
	m_Camera = CreateRef<Camera>();
	m_Simulation = CreateRef<Simulation>();
}

bool Editor::Initialize() {
	Renderer::Initialize();
	InitializePanels();
	return true;
}

void Editor::Update(Timestep ts) {
	DrawMenuBar();
    UpdatePanels();
	m_Camera->OnUpdate(ts);
	ProcessInput();
}

void Editor::Shutdown() {

}

void Editor::OnEvent(Event& e) {
    m_Camera->OnEvent(e);
}

void Editor::Render() {
	Renderer::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
	Renderer::Clear();
	Renderer::BeginScene(*m_Camera);

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

	Renderer::DrawLine({{-8.0f, -8.0f, -8.0f}, {-8.0f, 8.0f, -8.0f}});
	Renderer::DrawLine({{-8.0f, 8.0f, -8.0f}, {8.0f, 8.0f, -8.0f}});
	Renderer::DrawLine({{8.0f, 8.0f, -8.0f}, {8.0f, -8.0f, -8.0f}});
	Renderer::DrawLine({{8.0f, -8.0f, -8.0f}, {-8.0f, -8.0f, -8.0f}});

	Renderer::DrawLine({{-8.0f, -8.0f, 8.0f}, {-8.0f, 8.0f, 8.0f}});
	Renderer::DrawLine({{-8.0f, 8.0f, 8.0f}, {8.0f, 8.0f, 8.0f}});
	Renderer::DrawLine({{8.0f, 8.0f, 8.0f}, {8.0f, -8.0f, 8.0f}});
	Renderer::DrawLine({{8.0f, -8.0f, 8.0f}, {-8.0f, -8.0f, 8.0f}});
    
	Renderer::DrawLine({{-8.0f, -8.0f, -8.0f}, {-8.0f, -8.0f, 8.0f}});
	Renderer::DrawLine({{-8.0f, 8.0f, -8.0f}, {-8.0f, 8.0f, 8.0f}});
	Renderer::DrawLine({{8.0f, 8.0f, -8.0f}, {8.0f, 8.0f, 8.0f}});
	Renderer::DrawLine({{8.0f, -8.0f, -8.0f}, {8.0f, -8.0f, 8.0f}});

	Renderer::DrawLine({{-8.0f, -8.0f, 8.0f}, {-5.0f, -5.0f, 5.0f}});
	Renderer::DrawLine({{-8.0f, 8.0f, 8.0f}, {-5.0f, 5.0f, 5.0f}});
	Renderer::DrawLine({{8.0f, 8.0f, 8.0f}, {5.0f, 5.0f, 5.0f}});
	Renderer::DrawLine({{8.0f, -8.0f, 8.0f}, {5.0f, -5.0f, 5.0f}});
	
	Renderer::DrawLine({{-8.0f, -8.0f, -8.0f}, {-5.0f, -5.0f, -5.0f}});
	Renderer::DrawLine({{-8.0f, 8.0f, -8.0f}, {-5.0f, 5.0f, -5.0f}});
	Renderer::DrawLine({{8.0f, 8.0f, -8.0f}, {5.0f, 5.0f, -5.0f}});
	Renderer::DrawLine({{8.0f, -8.0f, -8.0f}, {5.0f, -5.0f, -5.0f}});

	for (int i = 0; i < 90; i++) {
		Circle circle;
		circle.Scale = {10, 10, 1.0f};
		//circle.Thickness = 0.5f;
		//circle.Fade = 0.5f;
		circle.CircleColor = {0.2f/180.0f * i, 0.3f/180.0f * i, 0.7f/180.0f * i, 1.0f};
		circle.Rotation = {glm::radians((float)i), 0.0f, 0.0f};
		Renderer::DrawCircle(circle);
	}

	for (int i = 0; i < 90; i++) {
		Circle circle;
		circle.Scale = {10, 10, 1.0f};
		//circle.Thickness = 0.5f;
		//circle.Fade = 0.5f;
		circle.CircleColor = {0.3f/180.0f * i, 0.7f/180.0f * i, 0.2f/180.0f * i, 1.0f};
		circle.Rotation = {glm::radians((float)(i + 90)), 0.0f, 0.0f};
		Renderer::DrawCircle(circle);
	}

	for (int i = 0; i < 90; i++) {
		Circle circle;
		circle.Scale = {10, 10, 1.0f};
		//circle.Thickness = 0.5f;
		//circle.Fade = 0.5f;
		circle.CircleColor = {0.7f/180.0f * i, 0.3f/180.0f * i, 0.2f/180.0f * i, 1.0f};
		circle.Rotation = {0.0f, glm::radians((float)i), 0.0f};
		Renderer::DrawCircle(circle);
	}

	for (int i = 0; i < 90; i++) {
		Circle circle;
		circle.Scale = {10, 10, 1.0f};
		//circle.Thickness = 0.5f;
		//circle.Fade = 0.5f;
		circle.CircleColor = {0.7f/180.0f * i, 0.7f/180.0f * i, 0.7f/180.0f * i, 1.0f};
		circle.Rotation = {0.0f, glm::radians((float)i), 0.0f};
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
		if (ImGui::BeginMenu("Simulation")) {
			if (ImGui::MenuItem("New", "Ctrl+N"))
				m_Prompt = EditorPrompts::NEW;
			if (ImGui::MenuItem("Open", "Ctrl+O"))
				m_Prompt = EditorPrompts::OPEN;
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				m_Prompt = EditorPrompts::SAVE;
			if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
				m_Prompt = EditorPrompts::SAVEAS;
			if (ImGui::MenuItem("Settings", "Ctrl+E"))
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

void Editor::ProcessInput() {
	if (Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl)) {
		if (m_InputPrimer) {
			if (Input::IsKeyPressed(KeyCode::N)) {
				m_Prompt = EditorPrompts::NEW;
				m_InputPrimer = false;
			} else if (Input::IsKeyPressed(KeyCode::O)) {
				m_Prompt = EditorPrompts::OPEN;
				m_InputPrimer = false;
			} else if (Input::IsKeyPressed(KeyCode::S)) {
				if (Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift)) {
					m_Prompt = EditorPrompts::SAVEAS;
				} else {
					m_Prompt = EditorPrompts::SAVE;
				}
				m_InputPrimer = false;
			} else if (Input::IsKeyPressed(KeyCode::E)) {
				FATAL("ERROR: NOT IMPLEMENTED");
				m_InputPrimer = false;
			}
		}
	} else {
		m_InputPrimer = true;
	}
}