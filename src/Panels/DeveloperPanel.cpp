#include "DeveloperPanel.h"
#include "imgui.h"

DeveloperPanel::DeveloperPanel() {

}

void DeveloperPanel::Initialize() {
}

void DeveloperPanel::Update(Forge* context) {
	const ImGuiTreeNodeFlags treeNodeFlags = 
		ImGuiTreeNodeFlags_SpanAvailWidth | 
		ImGuiTreeNodeFlags_AllowItemOverlap | 
		ImGuiTreeNodeFlags_FramePadding | 
		ImGuiTreeNodeFlags_DefaultOpen;

	ImGui::Begin("Developer Tools");

	if (ImGui::TreeNodeEx("Renderer Testing", treeNodeFlags)) {
		if (ImGui::TreeNodeEx("Lines", treeNodeFlags)) {
			ImGui::Text("Worthless crooft");
			static float s_p1, s_p2, s_p3, s_p4, s_p5, s_p6;
			ImGui::Columns(3);
			ImGui::DragFloat("##Point1x", &s_p1);
			ImGui::DragFloat("##Point1y", &s_p2);
			ImGui::DragFloat("##Point1z", &s_p3);
			ImGui::NextColumn();
			ImGui::Text("to");	
			ImGui::NextColumn();
			ImGui::DragFloat("##Point2x", &s_p4);
			ImGui::DragFloat("##Point2y", &s_p5);
			ImGui::DragFloat("##Point2z", &s_p6);
			ImGui::Columns(1);
			ImGui::Button("Add Line", {-1, 0});
			ImGui::TreePop();
		}
		ImGui::Button("Clear Artifacts", {-1, 0});
		ImGui::TreePop();
	}

	ImGui::End();
}
