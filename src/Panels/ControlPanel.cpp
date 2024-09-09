#include "ControlPanel.h"
#include "Core/Editor.h"
#include "imgui.h"

ControlPanel::ControlPanel() {
	m_Name = "Simulation Properties";
}

void ControlPanel::Update(Editor* context) {
	ImGui::Text("Bello!");   
}
