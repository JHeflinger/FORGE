#include "OverviewPanel.h"
#include "imgui.h"

OverviewPanel::OverviewPanel() {
	m_Name = "Simulation Overview";
}

void OverviewPanel::Initialize() {
}

void OverviewPanel::Update(Editor* context) {
	const ImGuiTreeNodeFlags treeNodeFlags = 
		ImGuiTreeNodeFlags_SpanAvailWidth | 
		ImGuiTreeNodeFlags_AllowItemOverlap | 
		ImGuiTreeNodeFlags_FramePadding | 
		ImGuiTreeNodeFlags_DefaultOpen;

	if (ImGui::TreeNodeEx("Sources", treeNodeFlags)) {

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Sinks", treeNodeFlags)) {

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Grid", treeNodeFlags)) {

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Particles", treeNodeFlags)) {

		ImGui::TreePop();
	}
}
