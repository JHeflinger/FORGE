#include "Editor.h"
#include "Renderer/Renderer.h"
#include "Panels/ViewportPanel.h"
#include "Panels/OverviewPanel.h"
#include "Panels/ResourcePanel.h"
#include "Panels/PlaybackPanel.h"
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
		CreateRef<PlaybackPanel>(),
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
	if (m_PlaybackStarted)
		DrawPlaybackParticles();
	else
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
			if (ImGui::MenuItem("Join", "Ctrl+J"))
				m_Prompt = EditorPrompts::JOIN;
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
		ImGui::Text("%s", lastSaved.c_str());
		ImGui::PopStyleColor();
		ImGui::EndMenuBar();
	}
}

void Editor::ProcessInput() {
	if (m_Prompt != EditorPrompts::NONE) return;
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
			} else if (Input::IsKeyPressed(KeyCode::J)) {
				m_Prompt = EditorPrompts::JOIN;
				m_InputPrimer = false;
			}
		}
	} else {
		m_InputPrimer = true;
	}
}

void Editor::DrawPrompts() {
	static int s_substate = 0;
	static char s_ip_addr_buffer[128] = "198.168.68.126";
	static char s_port_buffer[6] = "50051";
	static int s_local_workers = 1;
	static bool s_connection_failure = false;
	static SimulationDetails s_remote_details = { 0 };
	const char* length_units[] = { "ticks", "us", "ms", "s" };
	const char* solver_options[] = { "RKF45", "Euler", "LeapFrog" };
	char tbuffer[2048];
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];
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
		ImGui::SetNextWindowSize({600, 0});
		if (ImGui::BeginPopupModal("Run Simulation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    		float gapsize = 8.0f;
			std::vector<ClientMetadata> clients = m_Simulation->Clients();
			size_t connected = 0;
			ImGui::SetItemDefaultFocus();
			switch (s_substate) {
				case 0:
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Confirm Simulation Details").x / 2.0));
					ImGui::Text("Confirm Simulation Details");
					ImGui::Dummy({0, 2});
					ImGui::Separator();
					ImGui::Columns(2);
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
					snprintf(tbuffer, 2048, "%llu %s", (long long unsigned int)m_Simulation->Length(), length_units[(int)m_Simulation->LengthUnit()]);
					ImGui::Text("%s", tbuffer);
					ImGui::Dummy({0, gapsize});
					if (m_Simulation->SafeguardCacheEnabled()) {
						snprintf(tbuffer, 2048, "TRUE");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
					} else {
						snprintf(tbuffer, 2048, "FALSE");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155,155,155,255));
					}
					ImGui::Text("%s", tbuffer);
					ImGui::PopStyleColor();
					ImGui::Dummy({0, gapsize});
					if (m_Simulation->SimulationRecordEnabled()) {
						snprintf(tbuffer, 2048, "TRUE");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
					} else {
						snprintf(tbuffer, 2048, "FALSE");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155,155,155,255));
					}
					ImGui::Text("%s", tbuffer);
					ImGui::PopStyleColor();
					ImGui::Dummy({0, gapsize});
					ImGui::Text("%s", solver_options[(int)m_Simulation->Solver()]);
					ImGui::Dummy({0, gapsize});
					snprintf(tbuffer, 2048, "%.3f by %.3f", m_Simulation->Bounds().x, m_Simulation->Bounds().y);
					ImGui::Text("%s", tbuffer);
					ImGui::Dummy({0, gapsize});
					if (m_Simulation->DynamicTimestep())
						snprintf(tbuffer, 2048, "DYNAMIC");
					else
						snprintf(tbuffer, 2048, "%llu %s", (long long unsigned int)m_Simulation->Timestep(), length_units[(int)m_Simulation->LengthUnit()]);
					ImGui::Text("%s", tbuffer);
					ImGui::Dummy({0, gapsize});
					snprintf(tbuffer, 2048, "%lu", (long unsigned int)m_Simulation->NumLocalWorkers());
					ImGui::Text("%s", tbuffer);
					ImGui::Dummy({0, gapsize});
					snprintf(tbuffer, 2048, "%lu", (long unsigned int)m_Simulation->NumRemoteWorkers());
					ImGui::Text("%s", tbuffer);
					ImGui::Columns(1);
					ImGui::Dummy({0, 2});
					ImGui::Separator();
					break;
				case 1:
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Connect Remote Workers").x / 2.0));
					ImGui::Text("Connect Remote Workers");
					ImGui::Dummy({0, 2});
					ImGui::Separator();
					ImGui::Dummy({0, 2});
					for (size_t i = 0; i < clients.size(); i++) {
						if (clients[i].connected) connected++;
					}
					snprintf(tbuffer, 2048, "%lu/%lu Workers Connected", (long unsigned int)connected, (long unsigned int)m_Simulation->NumRemoteWorkers());
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize(tbuffer).x / 2.0f));
					ImGui::Text("%s", tbuffer);
					ImGui::Dummy({0, 2});
					ImGui::BeginChild("rworkers", {0, 300}, true, ImGuiWindowFlags_HorizontalScrollbar);
					for (size_t i = 0; i < clients.size(); i++) {
						float ypos = ImGui::GetCursorPosY();
						ImGui::Text("%s", clients[i].ip.c_str());
						ImGui::SetCursorPosY(ypos);
						if (!clients[i].connected) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
							ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("waiting for connection").x);
							ImGui::Text("waiting for connection");
						} else {
							if (!clients[i].ready) {
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
								ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("verifying").x);
								ImGui::Text("verifying");
							} else {
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
								ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("connected").x);
								ImGui::Text("connected");
							}
						}
						ImGui::PopStyleColor();
					}
					ImGui::EndChild();
					break;
				case 2:
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Execute Simulation").x / 2.0));
					ImGui::Text("Execute Simulation");
					ImGui::Dummy({0, 2});
					ImGui::Separator();
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Simulation Log").x / 2.0f));
					ImGui::Text("Simulation Log");
					ImGui::Dummy({0, 2});
					ImGui::BeginChild("logs", {0, 300}, true, ImGuiWindowFlags_HorizontalScrollbar);
					for (size_t i = 0; i < m_Simulation->Logs().size(); i++)
						ImGui::Text("%s", m_Simulation->Logs()[i].c_str());
					ImGui::EndChild();
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Simulation Progress").x / 2.0f));
					ImGui::Text("Simulation Progress");
					ImGui::Dummy({0, 2});
    				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.7f, 0.4f, 1.0f));
					ImGui::PushFont(boldFont);
					ImGui::ProgressBar(m_Simulation->Progress(), ImVec2(-1.0f, 0.0f), nullptr);
					ImGui::PopStyleColor();
					ImGui::PopFont();
					ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 53);
					if (m_Simulation->Finished())
						ImGui::BeginDisabled();
					if (!m_Simulation->Started()) {
						if (ImGui::Button("START", {60, 25})) {
							if (m_Simulation->NumRemoteWorkers() > 0) m_Simulation->StartRemote();
							else m_Simulation->StartLocal();
						}
					} else if (m_Simulation->Paused()) {
						if (ImGui::Button("RESUME", {60, 25})) {
							m_Simulation->Resume();
						}
					} else {
						if (ImGui::Button("PAUSE", {60, 25})) {
							m_Simulation->Pause();
						}
					}
					if (m_Simulation->Finished())
						ImGui::EndDisabled();
					ImGui::SameLine();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 135);
					if (!m_Simulation->Started())
						ImGui::BeginDisabled();
					if (ImGui::Button("ABORT", {60, 25})) {
						m_Simulation->Abort();
					} else if (!m_Simulation->Started())
						ImGui::EndDisabled();
					ImGui::Separator();
					break;
				default: break;
			}
			
			ImGui::Dummy({0, 10});
			if ((s_substate == 2 && m_Simulation->Started()) || (s_substate == 2 && m_Simulation->Finished()))
				ImGui::BeginDisabled();
			if (ImGui::Button("Cancel", {60, 25})) {
				m_Prompt = EditorPrompts::NONE;
				s_substate = 0; 
				ImGui::CloseCurrentPopup();
				// TODO: shut down network service
			}
			if ((s_substate == 2 && m_Simulation->Started()) || (s_substate == 2 && m_Simulation->Finished()))
				ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x + 15);
			if (s_substate != 2) {
				bool disabled = false;
				if (s_substate == 1 && connected != m_Simulation->NumRemoteWorkers()) disabled = true;
				if (disabled) ImGui::BeginDisabled();
				if (ImGui::Button("Next", {60, 25})) {
					s_substate++;
					if (s_substate == 2) {
						m_Simulation->Prime();
					} else if (s_substate == 1 && m_Simulation->NumRemoteWorkers() == 0) {
						s_substate++;
					} else if (s_substate == 1) {
						m_Simulation->Host();
					}
				}
				if (disabled) ImGui::EndDisabled();
			} else {
				if (!m_Simulation->Finished())
					ImGui::BeginDisabled();
				if (ImGui::Button("Finish", {60, 25})) {
					m_Prompt = EditorPrompts::NONE;
					s_substate = 0; 
					ImGui::CloseCurrentPopup();
					// TODO: shut down network service
				}
				if (!m_Simulation->Finished())
					ImGui::EndDisabled();
				if (m_Simulation->Started() && !m_Simulation->Paused()) {
					m_Simulation->Checkup();
				}
			}
			if (s_substate > 0) {
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 135);
				if ((s_substate == 2 && m_Simulation->Started()) || (s_substate == 2 && m_Simulation->Finished()))
					ImGui::BeginDisabled();
				if (ImGui::Button("Back", {60, 25})) {
					s_substate--; 
					if (s_substate == 1 && m_Simulation->NumRemoteWorkers() == 0) {
						s_substate--;
					}
					// TODO: shut down network service
				}
				if ((s_substate == 2 && m_Simulation->Started()) || (s_substate == 2 && m_Simulation->Finished()))
					ImGui::EndDisabled();
			}
			ImGui::EndPopup();
		}
		break;
	case EditorPrompts::JOIN:
		ImGui::OpenPopup("Join Simulation");
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize({600, 0});
		if (ImGui::BeginPopupModal("Join Simulation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    		float gapsize = 8.0f;
			ImGui::SetItemDefaultFocus();

			switch (s_substate) {
				case 0:
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Join a Remote Simulation").x / 2.0));
					ImGui::Text("Join a Remote Simulation");
					ImGui::Dummy({0, 2});
					ImGui::Separator();
					ImGui::Dummy({0, 2});
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 450);
					ImGui::Text("Simulation Host IP Address");
					ImGui::Dummy({0, gapsize});
					ImGui::Text("Simulation Host Port");
					ImGui::Dummy({0, gapsize});
					ImGui::Text("Available Workers");
					ImGui::NextColumn();
					ImGui::InputText("##host_ip", s_ip_addr_buffer, sizeof(s_ip_addr_buffer));
					ImGui::Dummy({0, 2});
					ImGui::InputText("##host_port", s_port_buffer, sizeof(s_port_buffer));
					ImGui::Dummy({0, 2});
					ImGui::DragInt("##remotelocalworkers", &s_local_workers, 1, 1, INT_MAX);
					ImGui::Columns(1);
					if (s_connection_failure) {
						ImGui::Dummy({0, 10});
						ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Unable to connect").x / 2.0));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
						ImGui::Text("Unable to connect");
						ImGui::PopStyleColor();
						ImGui::Dummy({0, 132 - ImGui::CalcTextSize("Unable to connect").y});
					} else {
						ImGui::Dummy({0, 150});
					}
					ImGui::Separator();
					break;
				case 1:
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Verify Simulation Details").x / 2.0));
					ImGui::Text("Verify Simulation Details");
					ImGui::Dummy({0, 2});
					ImGui::Separator();
					ImGui::Columns(2);
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
					ImGui::NextColumn();
					ImGui::SetColumnWidth(0, 200);
					snprintf(tbuffer, 2048, "%llu %s", (long long unsigned int)s_remote_details.length, length_units[s_remote_details.unit]);
					ImGui::Text("%s", tbuffer);
					ImGui::Dummy({0, gapsize});
					if (s_remote_details.safeguard) {
						snprintf(tbuffer, 2048, "TRUE");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
					} else {
						snprintf(tbuffer, 2048, "FALSE");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155,155,155,255));
					}
					ImGui::Text("%s", tbuffer);
					ImGui::PopStyleColor();
					ImGui::Dummy({0, gapsize});
					if (s_remote_details.record) {
						snprintf(tbuffer, 2048, "TRUE");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
					} else {
						snprintf(tbuffer, 2048, "FALSE");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(155,155,155,255));
					}
					ImGui::Text("%s", tbuffer);
					ImGui::PopStyleColor();
					ImGui::Dummy({0, gapsize});
					ImGui::Text("%s", solver_options[s_remote_details.solver]);
					ImGui::Dummy({0, gapsize});
					snprintf(tbuffer, 2048, "%.3f by %.3f", s_remote_details.boundx, s_remote_details.boundy);
					ImGui::Text("%s", tbuffer);
					ImGui::Dummy({0, gapsize});
					snprintf(tbuffer, 2048, "%llu %s", (long long unsigned int)s_remote_details.timestep, length_units[s_remote_details.unit]);
					ImGui::Text("%s", tbuffer);
					ImGui::Columns(1);
					ImGui::Dummy({0, 2});
					ImGui::Separator();
					break;
				case 2:
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Monitor Simulation").x / 2.0));
					ImGui::Text("Monitor Simulation");
					ImGui::Dummy({0, 2});
					ImGui::Separator();
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Simulation Log").x / 2.0f));
					ImGui::Text("Simulation Log");
					ImGui::Dummy({0, 2});
					ImGui::BeginChild("logs", {0, 300}, true, ImGuiWindowFlags_HorizontalScrollbar);
					for (size_t i = 0; i < m_Simulation->Logs().size(); i++)
						ImGui::Text("%s", m_Simulation->Logs()[i].c_str());
					ImGui::EndChild();
					ImGui::Dummy({0, 2});
					ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2.0f) - (ImGui::CalcTextSize("Simulation Progress").x / 2.0f));
					ImGui::Text("Simulation Progress");
					ImGui::Dummy({0, 2});
    				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.7f, 0.4f, 1.0f));
					ImGui::PushFont(boldFont);
					ImGui::ProgressBar(m_Simulation->Progress(), ImVec2(-1.0f, 0.0f), nullptr);
					ImGui::PopStyleColor();
					ImGui::PopFont();
					ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 53);
					ImGui::Dummy({0, 10});
					ImGui::Separator();
					break;
				default: break;
			}

			ImGui::Dummy({0, 10});
			if (ImGui::Button("Cancel", {60, 25})) {
				m_Prompt = EditorPrompts::NONE;
				s_substate = 0; 
				ImGui::CloseCurrentPopup();
				// TODO: shut down network service
			}			
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x + 15);
			if (s_substate == 0) {
				if (ImGui::Button("Connect", {60, 25})) {
					std::string ipaddr(s_ip_addr_buffer);
					std::string port(s_port_buffer);
					if (m_Simulation->Connect(ipaddr, port, s_local_workers, &s_remote_details)) {
						s_connection_failure = false;
						s_substate++;
					} else {
						s_connection_failure = true;
					}
				}
			} else if (s_substate == 1) {
				if (ImGui::Button("Verify", {60, 25})) {
					if (m_Simulation->Verify()) {
						m_Simulation->Communicate(); // TODO: combine verify and communicate to reduce possibilty of losing a client in between calls
						s_substate++;
					}
				}
			} else if (s_substate == 2) {
				if (!m_Simulation->Finished())
					ImGui::BeginDisabled();
				if (ImGui::Button("Finish", {60, 25})) {
					m_Prompt = EditorPrompts::NONE;
					s_substate = 0; 
					ImGui::CloseCurrentPopup();
					// TODO: shut down network service
				}
				if (!m_Simulation->Finished())
					ImGui::EndDisabled();
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
			(glm::vec3)particle->Position(),
			(float)particle->Radius()
		});
	}
}

void Editor::DrawPlaybackParticles() {
	if (m_Simulation->SimulationRecord().size() <= (uint64_t)m_PlaybackFrameTime) {
		m_PlaybackStarted = false;
		m_PlaybackState = EditorPlaybackState::READY;
		return;
	}
	for (Particle particle : m_Simulation->SimulationRecord()[(size_t)m_PlaybackFrameTime]) {
		Renderer::DrawSphere({
			(glm::vec3)particle.Position(),
			(float)particle.Radius()
		});
	}
	if (m_PlaybackState != EditorPlaybackState::PAUSED)
		m_PlaybackFrameTime += m_PlaybackSpeed;
}

void Editor::StartPlayback() {
	m_PlaybackFrameTime = 0.0f;
	m_PlaybackStarted = true;
	m_PlaybackState = EditorPlaybackState::STARTED;
}

void Editor::StopPlayback() {
	m_PlaybackState = EditorPlaybackState::READY;
	m_PlaybackStarted = false;
}

void Editor::PausePlayback() {
	m_PlaybackState = EditorPlaybackState::PAUSED;
}

void Editor::ResumePlayback() {
	m_PlaybackState = EditorPlaybackState::STARTED;
}

void Editor::StepPlayback(int steps) {
	m_PlaybackFrameTime += (float)steps;
	if (m_PlaybackFrameTime < 0.0f) m_PlaybackFrameTime = 0.0f;
	if (m_PlaybackFrameTime > m_Simulation->SimulationRecord().size()) m_PlaybackFrameTime = m_Simulation->SimulationRecord().size();
}

float Editor::PlaybackProgression() {
	if (m_Simulation->SimulationRecord().size() == 0) return 0.0f;
	if (m_PlaybackState == EditorPlaybackState::READY) return 1.0f;
	return (float)(m_PlaybackFrameTime/m_Simulation->SimulationRecord().size());
}
