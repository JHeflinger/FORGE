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
    float gapsize = 8.0f;
    ImGui::Columns(2);
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Simulation Length");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Safeguard Cache");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Simulation Record");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Simulation Solver");
    ImGui::Dummy({0, gapsize});
    ImGui::NextColumn();
    gapsize = 2.0f;
    ImGui::Dummy({0, gapsize});
	uint64_t simulationlength = context->GetSimulation()->Length();
	if (ImGui::DragScalar("##simulationlength", ImGuiDataType_U64, &simulationlength, 1.0f))
		context->GetSimulation()->SetLength(simulationlength);
	ImGui::SameLine();
    const char* length_units[] = { "ticks", "us", "ms", "s" };
	int current_length_unit = (int)context->GetSimulation()->LengthUnit();
    if (ImGui::Combo("##lengthunits", &current_length_unit, length_units, IM_ARRAYSIZE(length_units)))
		context->GetSimulation()->SetLengthUnit((SimulationLengthUnit)current_length_unit);
    ImGui::Dummy({0, gapsize});
	bool checkbox = context->GetSimulation()->SafeguardCacheEnabled();
    if (ImGui::Checkbox("##safeguardcache", &checkbox))
		context->GetSimulation()->SetSafeguardCache(checkbox);
	checkbox = context->GetSimulation()->SimulationRecordEnabled();
    ImGui::Dummy({0, gapsize});
    if (ImGui::Checkbox("##simulationrecord", &checkbox))
		context->GetSimulation()->SetSimulationRecord(checkbox);
    ImGui::Dummy({0, gapsize});
	int current_solver = (int)context->GetSimulation()->Solver();
    const char* solver_options[] = { "RKF45", "Euler" };
    if (ImGui::Combo("##simulationsolver", &current_solver, solver_options, IM_ARRAYSIZE(solver_options)))
		context->GetSimulation()->SetSolver((SimulationSolver)current_solver);
}
