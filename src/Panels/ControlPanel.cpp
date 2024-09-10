#include "ControlPanel.h"
#include "Core/Editor.h"
#include "Core/Log.h"
#include "imgui.h"

ControlPanel::ControlPanel() {
	m_Name = "Simulation Properties";
}

void ControlPanel::Initialize() {
	WARN("Currently there is no substantial implementation for"
		"\n\t- Simulation length"
		"\n\t- Simulation cache settings"
		"\n\t- Simulation recording"
		"\n\t- Simulation solver");
}

void ControlPanel::Update(Editor* context) {
	ImGui::Text("Bello!");   
	// Simulation length | █# of units█ █unit type dropdown█
	// Safeguard Cache   | ■ Enabled
	// Simulation Record | ■ Enabled
	// Simulation Solver | █solver dropdown█
}
