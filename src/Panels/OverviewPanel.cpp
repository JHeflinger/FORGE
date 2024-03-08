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
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24 * 0.5f);
	if (ImGui::Button("+##addsource", ImVec2{ 24, 24 }))
		ImGui::OpenPopup("Add Source");
	if (ImGui::BeginPopup("Add Source")) {
		if (ImGui::MenuItem("New Source")) {
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Clear Sources")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::Separator();

	if (ImGui::TreeNodeEx("Sinks", treeNodeFlags)) {

		ImGui::TreePop();
	}
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24 * 0.5f);
	if (ImGui::Button("+##addsink", ImVec2{ 24, 24 }))
		ImGui::OpenPopup("Add Sink");
	if (ImGui::BeginPopup("Add Sink")) {
		if (ImGui::MenuItem("New Sink")) {
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Clear Sinks")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::Separator();

	if (ImGui::TreeNodeEx("Grid", treeNodeFlags)) {

		ImGui::TreePop();
	}
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24 * 0.5f);
	if (ImGui::Button("+##addgrid", ImVec2{ 24, 24 }))
		ImGui::OpenPopup("Add Grid");
	if (ImGui::BeginPopup("Add Grid")) {
		if (ImGui::MenuItem("New Grid")) {
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Clear Grids")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::Separator();

	if (ImGui::TreeNodeEx("Particles", treeNodeFlags)) {

		ImGui::TreePop();
	}
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24 * 0.5f);
	if (ImGui::Button("+##addparticle", ImVec2{ 24, 24 }))
		ImGui::OpenPopup("Add Particle");
	if (ImGui::BeginPopup("Add Particle")) {
		if (ImGui::MenuItem("New Particle")) {
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Clear Particles")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
