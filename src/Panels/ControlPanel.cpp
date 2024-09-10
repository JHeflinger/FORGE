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
	static uint64_t temp = 0;
	static bool temp1 = false;
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
	ImGui::DragScalar("##simulationlength", ImGuiDataType_U64, &temp, 1.0f);
	ImGui::SameLine();
	static int current_item_0 = 0;
    const char* items_0[] = { "ticks", "us", "ms", "s" };
    ImGui::Combo("##lengthunits", &current_item_0, items_0, IM_ARRAYSIZE(items_0));
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##safeguardcache", &temp1);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##simulationrecord", &temp1);
    ImGui::Dummy({0, gapsize});
	static int current_item = 0;
    const char* items[] = { "rkh45", "coolsolver", "???", "footbar" };
    ImGui::Combo("##simulationsolver", &current_item, items, IM_ARRAYSIZE(items));
}
