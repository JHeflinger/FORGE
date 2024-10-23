#include "Editor.h"
#include "Renderer/Renderer.h"
#include "Panels/ViewportPanel.h"
#include "Panels/OverviewPanel.h"
#include "Panels/ResourcePanel.h"
#include "Panels/ControlPanel.h"
#include "Panels/PlanePanel.h"
#include "Events/Input.h"
#include "Utils/FileUtils.h"
#include "Utils/MathUtils.h"
#include "imgui.h"
#include <cmath>

Editor::Editor() {
    m_Panels = { 
		CreateRef<ViewportPanel>(), 
		CreateRef<OverviewPanel>(),
		CreateRef<ResourcePanel>(),
		CreateRef<PlanePanel>(),
		CreateRef<ControlPanel>(),
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
	m_LastSavedTime += ts.GetSeconds();
	ProcessInput();
	DrawPrompts();
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
	DrawStaticParticles();
	for (auto panel : m_Panels) {
        panel->Render();
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
			if (ImGui::MenuItem("New", "Ctrl+N")) {
				m_SimulationSaved = Serializer::SimulationSaved(m_Simulation);
				m_Prompt = EditorPrompts::NEW;
			}
			if (ImGui::MenuItem("Open", "Ctrl+O")) {
				m_SimulationSaved = Serializer::SimulationSaved(m_Simulation);
				m_Prompt = EditorPrompts::OPEN;
			}
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				m_Prompt = EditorPrompts::SAVE;
			if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
				m_Prompt = EditorPrompts::SAVEAS;
			if (ImGui::MenuItem("Run", "Ctrl+R"))
				m_Prompt = EditorPrompts::RUN;
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Panels")) {
			for (auto panel : m_Panels) {
				ImGui::MenuItem(panel->Name().c_str(), NULL, &(panel->m_Enabled));
			}
			ImGui::EndMenu();
		}
		ImGui::SameLine(ImGui::GetWindowSize().x);
		std::string lastSaved = "Last Saved " + GetLastSavedString() + " ago";
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::CalcTextSize(lastSaved.c_str()).x - 5);
		ImGui::PushStyleColor(ImGuiCol_Text, {0.6, 0.6, 0.6, 1});
		ImGui::Text(lastSaved.c_str());
		ImGui::PopStyleColor();
		ImGui::EndMenuBar();
	}
}

void Editor::ProcessInput() {
	if (Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl)) {
		if (m_InputPrimer) {
			if (Input::IsKeyPressed(KeyCode::N)) {
				m_SimulationSaved = Serializer::SimulationSaved(m_Simulation);
				m_Prompt = EditorPrompts::NEW;
				m_InputPrimer = false;
			} else if (Input::IsKeyPressed(KeyCode::O)) {
				m_SimulationSaved = Serializer::SimulationSaved(m_Simulation);
				m_Prompt = EditorPrompts::OPEN;
				m_InputPrimer = false;
			} else if (Input::IsKeyPressed(KeyCode::S)) {
				if (Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift)) {
					m_Prompt = EditorPrompts::SAVEAS;
				} else {
					m_Prompt = EditorPrompts::SAVE;
				}
				m_InputPrimer = false;
			} else if (Input::IsKeyPressed(KeyCode::R)) {
				m_Prompt = EditorPrompts::RUN;
				m_InputPrimer = false;
			}
		}
	} else {
		m_InputPrimer = true;
	}
}

void Editor::DrawPrompts() {
	switch (m_Prompt) {
	case EditorPrompts::NONE: return;
	case EditorPrompts::NEW:
		if (!m_SimulationSaved) {
			ImGui::OpenPopup("WARNING");
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("WARNING", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::SetItemDefaultFocus();
				ImGui::Text("You have not saved this scene! Would you like to save first?\n\n");
				ImGui::Separator();
				ImGui::Dummy({ 0, 3 });
				if (ImGui::Button("YES", {60, 25})) {
					if (FileUtils::Save(Serializer::SerializeSimulation(m_Simulation), m_Simulation->Filepath()) == "") {
						m_Prompt = EditorPrompts::NONE;
						m_LastSavedTime = 0.0f;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("NO", {60, 25})) {
					m_Simulation = CreateRef<Simulation>();
					m_Camera->SoftReset();
					m_Prompt = EditorPrompts::NONE;
					m_LastSavedTime = 0.0f;
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x - 68);
				if (ImGui::Button("Cancel", {60, 25})) {
					m_Prompt = EditorPrompts::NONE;
					m_LastSavedTime = 0.0f;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		} else {
			m_Simulation = CreateRef<Simulation>();
			m_Camera->SoftReset();
			m_LastSavedTime = 0.0f;
			m_Prompt = EditorPrompts::NONE;
		}
		break;
	case EditorPrompts::OPEN:
		if (!m_SimulationSaved) {
			ImGui::OpenPopup("WARNING");
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("WARNING", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::SetItemDefaultFocus();
				ImGui::Text("You have not saved this scene! Would you like to save first?\n\n");
				ImGui::Separator();
				ImGui::Dummy({ 0, 3 });
				if (ImGui::Button("YES", {60, 25})) {
					if (FileUtils::Save(Serializer::SerializeSimulation(m_Simulation), m_Simulation->Filepath()) != "") {
						Serializer::DeserializeSimulation(m_Simulation, FileUtils::Open());
						m_LastSavedTime = 0.0f;
						m_Prompt = EditorPrompts::NONE;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("NO", {60, 25})) {
					Serializer::DeserializeSimulation(m_Simulation, FileUtils::Open());
					m_LastSavedTime = 0.0f;
					m_Prompt = EditorPrompts::NONE;
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetWindowSize().x - 68);
				if (ImGui::Button("Cancel", {60, 25})) {
					m_LastSavedTime = 0.0f;
					m_Prompt = EditorPrompts::NONE;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		} else {
			Serializer::DeserializeSimulation(m_Simulation, FileUtils::Open());
			m_LastSavedTime = 0.0f;
			m_Prompt = EditorPrompts::NONE;
		}
		break;
	case EditorPrompts::SAVE:
		m_Simulation->SetFilepath(FileUtils::Save(Serializer::SerializeSimulation(m_Simulation), m_Simulation->Filepath()));
		m_Prompt = EditorPrompts::NONE;
		m_LastSavedTime = 0.0f;
		break;
	case EditorPrompts::SAVEAS:
		m_Simulation->SetFilepath(FileUtils::Save(Serializer::SerializeSimulation(m_Simulation)));
		m_Prompt = EditorPrompts::NONE;
		m_LastSavedTime = 0.0f;
		break;
	case EditorPrompts::RUN:
		ImGui::OpenPopup("Run Simulation");
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize({400, 0});
		if (ImGui::BeginPopupModal("Run Simulation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::SetItemDefaultFocus();
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Confirm Simulation Details").x / 2.0));
			ImGui::Text("Confirm Simulation Details");
			ImGui::Separator();
			ImGui::Columns(2);
    		float gapsize = 8.0f;
			ImGui::SetColumnWidth(0, 200);
			ImGui::Text("Simulation Length");
    		ImGui::Dummy({0, gapsize});
			ImGui::Text("Safeguard Cache Enabled");
    		ImGui::Dummy({0, gapsize});
			ImGui::Text("Simulation Record Enabled");
    		ImGui::Dummy({0, gapsize});
			ImGui::Text("Simulation Solver");
    		ImGui::Dummy({0, gapsize});
			ImGui::Text("Bounds");
    		ImGui::Dummy({0, gapsize});
			ImGui::Text("Timestep");
    		ImGui::Dummy({0, gapsize});
			ImGui::Text("Local Workers");
    		ImGui::Dummy({0, gapsize});
			ImGui::Text("Remote Workers");
			ImGui::NextColumn();
			ImGui::SetColumnWidth(0, 200);
			char tbuffer[2048];
			const char* length_units[] = { "ticks", "us", "ms", "s" };
			snprintf(tbuffer, 2048, "%llu %s", (long long unsigned int)m_Simulation->Length(), length_units[(int)m_Simulation->LengthUnit()]);
			ImGui::Text(tbuffer);
			ImGui::Dummy({0, gapsize});
			if (m_Simulation->SafeguardCacheEnabled()) {
				snprintf(tbuffer, 2048, "TRUE");
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
			} else {
				snprintf(tbuffer, 2048, "FALSE");
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155,155,155,255));
			}
			ImGui::Text(tbuffer);
			ImGui::PopStyleColor();
			ImGui::Dummy({0, gapsize});
			if (m_Simulation->SimulationRecordEnabled()) {
				snprintf(tbuffer, 2048, "TRUE");
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
			} else {
				snprintf(tbuffer, 2048, "FALSE");
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155,155,155,255));
			}
			ImGui::Text(tbuffer);
			ImGui::PopStyleColor();
			const char* solver_options[] = { "RKF45", "Euler", "LeapFrog" };
			ImGui::Dummy({0, gapsize});
			ImGui::Text(solver_options[(int)m_Simulation->Solver()]);
			ImGui::Dummy({0, gapsize});
			snprintf(tbuffer, 2048, "%.3f by %.3f", m_Simulation->Bounds().x, m_Simulation->Bounds().y);
			ImGui::Text(tbuffer);
			ImGui::Dummy({0, gapsize});
			if (m_Simulation->DynamicTimestep())
				snprintf(tbuffer, 2048, "DYNAMIC");
			else
				snprintf(tbuffer, 2048, "%llu %s", (long long unsigned int)m_Simulation->Timestep(), length_units[(int)m_Simulation->LengthUnit()]);
			ImGui::Text(tbuffer);
			ImGui::Dummy({0, gapsize});
			snprintf(tbuffer, 2048, "%lu", (long unsigned int)m_Simulation->NumLocalWorkers());
			ImGui::Text(tbuffer);
			ImGui::Dummy({0, gapsize});
			snprintf(tbuffer, 2048, "%lu", (long unsigned int)m_Simulation->NumRemoteWorkers());
			ImGui::Text(tbuffer);

			ImGui::Columns(1);
    		ImGui::Dummy({0, gapsize});
			if (ImGui::Button("Cancel", {60, 25})) {
				m_Prompt = EditorPrompts::NONE;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x + 15);
			if (ImGui::Button("Next", {60, 25})) {

			}
			ImGui::EndPopup();
		}
		break;
	default:
		FATAL("Invalid editor prompt detected!");
	}
}

std::string Editor::GetLastSavedString() {
	float time = m_LastSavedTime;
	std::stringstream ss;
	if (time < 60.0)
		ss << (int)time << " seconds";
	else if (time / 60.0f < 60.0)
		ss << (int)(time / 60.0f) << " minutes";
	else ss << (int)(time / 3600.0f) << " hours";
	return ss.str();
}

void Editor::DrawStaticParticles() {
	for (Ref<Particle> particle : m_Simulation->Particles()) {
		Renderer::DrawSphere({
			particle->Position(),
			particle->Radius()
		});
	}
}
