#include "HierarchyPanel.h"
#include "imgui.h"

HierarchyPanel::HierarchyPanel() {

}

void HierarchyPanel::Initialize() {
}

void HierarchyPanel::Update(Editor* context) {
	const ImGuiTreeNodeFlags treeNodeFlags = 
		ImGuiTreeNodeFlags_SpanAvailWidth | 
		ImGuiTreeNodeFlags_AllowItemOverlap | 
		ImGuiTreeNodeFlags_FramePadding | 
		ImGuiTreeNodeFlags_DefaultOpen;

	ImGui::Begin("Simulation Hierarchy");

	ImGui::End();
}
