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
		    "\n\t- Simulation solver"
		    "\n\t- Bounds"
		    "\n\t- Timestep"
		    "\n\t- Local Workers"
		    "\n\t- Remote Workers");
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
    ImGui::Text("Simulation Technique");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Bounds");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Timestep");
    ImGui::Dummy({0, gapsize});
    ImGui::Dummy({0, ImGui::CalcTextSize("Timestep").y});
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Local Workers");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Remote Workers");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Unit Size");
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
    const char* solver_options[] = { "RKF45", "Euler", "LeapFrog" };
    if (ImGui::Combo("##simulationsolver", &current_solver, solver_options, IM_ARRAYSIZE(solver_options)))
	    context->GetSimulation()->SetSolver((SimulationSolver)current_solver);
    ImGui::Dummy({0, gapsize});
	int current_technique = (int)context->GetSimulation()->Technique();
    const char* technique_options[] = { "Barnes-Hut", "Naive Particle", "Edge Distribution" };
    if (ImGui::Combo("##simulationtechnique", &current_technique, technique_options, IM_ARRAYSIZE(technique_options)))
	    context->GetSimulation()->SetTechnique((SimulationTechnique)current_technique);
    ImGui::Dummy({0, gapsize});
    double bounds_x = context->GetSimulation()->Bounds().x;
    double bounds_y = context->GetSimulation()->Bounds().y;
    bool bounds_set = false;
    float availableWidth = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("by").x;
    ImGui::SetNextItemWidth(availableWidth * 0.5f);
    if (ImGui::InputDouble("##bounds_x", &bounds_x, 0.0, 0.0, "%.3f"))
	    bounds_set = true;
    ImGui::SameLine();
    ImGui::Text("by");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(availableWidth * 0.5f);
    if (ImGui::InputDouble("##bounds_y", &bounds_y, 0.0, 0.0, "%.3f"))
        bounds_set = true;
    if (bounds_set)
	    context->GetSimulation()->SetBounds({bounds_x, bounds_y});
    ImGui::Dummy({0, gapsize});
    bool dynamic_timestep = context->GetSimulation()->DynamicTimestep();
    if (ImGui::Checkbox("Dynamic", &dynamic_timestep))
        context->GetSimulation()->SetDynamicTimestep(dynamic_timestep);
    ImGui::Dummy({0, gapsize});
    if (dynamic_timestep) {
        ImGui::BeginDisabled();
	}
	uint64_t timestep = context->GetSimulation()->Timestep();
	if (ImGui::DragScalar("##timestep", ImGuiDataType_U64, &timestep, 1.0f))
	    context->GetSimulation()->SetTimestep(timestep);
	ImGui::SameLine();
    ImGui::Text(length_units[current_length_unit]);
    if (dynamic_timestep) {
        ImGui::EndDisabled();
	}
    ImGui::Dummy({0, gapsize});
    uint32_t workers = context->GetSimulation()->NumLocalWorkers();
    if (ImGui::DragScalar("##localworkers", ImGuiDataType_U32, &workers, 1.0)) {
        context->GetSimulation()->SetNumLocalWorkers(workers);
	}
    ImGui::Dummy({0, gapsize});
    workers = context->GetSimulation()->NumRemoteWorkers();
    if (ImGui::DragScalar("##remoteworkers", ImGuiDataType_U32, &workers, 1.0)) {
        context->GetSimulation()->SetNumRemoteWorkers(workers);
	}
    ImGui::Dummy({0, gapsize});
	double unitsize = context->GetSimulation()->UnitSize();
    if (ImGui::InputDouble("##unitsize", &unitsize, 0.0, 0.0, "%.3f")) {
	    context->GetSimulation()->SetUnitSize(unitsize);
	}
}
