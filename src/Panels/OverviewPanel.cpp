#include "OverviewPanel.h"
#include "../Core/Editor.h"
#include "imgui.h"

#define WIDE_SEP() ImGui::Dummy({0, 1}); ImGui::Separator(); ImGui::Dummy({0, 1});

static void DrawAddSource(Editor* context, bool closed = true) {
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - (closed ? 12 : -9));
	if (ImGui::Button("+##addsource", ImVec2{ 24, 24 }))
		ImGui::OpenPopup("Add Source");
	if (ImGui::BeginPopup("Add Source")) {
		if (ImGui::MenuItem("New Source")) {
			context->GetSimulation()->Sources().push_back(CreateRef<Source>());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Clear Sources")) {
			context->GetSimulation()->Sources().clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

static void DrawAddSink(Editor* context, bool closed = true) {
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - (closed ? 12 : -9));
	if (ImGui::Button("+##addsink", ImVec2{ 24, 24 }))
		ImGui::OpenPopup("Add Sink");
	if (ImGui::BeginPopup("Add Sink")) {
		if (ImGui::MenuItem("New Sink")) {
			context->GetSimulation()->Sinks().push_back(CreateRef<Sink>());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Clear Sinks")) {
			context->GetSimulation()->Sinks().clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

static void DrawAddGrid(Editor* context, bool closed = true) {
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - (closed ? 12 : -9));
	if (ImGui::Button("+##addgrid", ImVec2{ 24, 24 }))
		ImGui::OpenPopup("Add Grid");
	if (ImGui::BeginPopup("Add Grid")) {
		if (ImGui::MenuItem("New Grid")) {
			context->GetSimulation()->Grids().push_back(CreateRef<Grid>());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Clear Grids")) {
			context->GetSimulation()->Grids().clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

static void DrawAddParticle(Editor* context, bool closed = true) {
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - (closed ? 12 : -9));
	if (ImGui::Button("+##addparticle", ImVec2{ 24, 24 }))
		ImGui::OpenPopup("Add Particle");
	if (ImGui::BeginPopup("Add Particle")) {
		if (ImGui::MenuItem("New Particle")) {
			context->GetSimulation()->Particles().push_back(CreateRef<Particle>());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("Clear Particles")) {
			context->GetSimulation()->Particles().clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

OverviewPanel::OverviewPanel() {
	m_Name = "Simulation Overview";
}

void OverviewPanel::Initialize() {
}

void OverviewPanel::Update(Editor* context) {
	uint64_t tid = 1;
	const ImGuiTreeNodeFlags treeNodeFlags = 
		ImGuiTreeNodeFlags_SpanAvailWidth | 
		ImGuiTreeNodeFlags_AllowItemOverlap | 
		ImGuiTreeNodeFlags_FramePadding | 
		ImGuiTreeNodeFlags_DefaultOpen;

	if (ImGui::TreeNodeEx("Sources", treeNodeFlags)) {
		DrawAddSource(context, false);
		for (Ref<Source> source : context->GetSimulation()->Sources()) {
			tid++;
			ImGuiTreeNodeFlags flags = (false ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(tid), flags, "Untitled Source")) {

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	} else DrawAddSource(context);
	WIDE_SEP();

	if (ImGui::TreeNodeEx("Sinks", treeNodeFlags)) {
		DrawAddSink(context, false);
		for (Ref<Sink> sink : context->GetSimulation()->Sinks()) {
			tid++;
			ImGuiTreeNodeFlags flags = (false ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(tid), flags, "Untitled Sink")) {

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	} else DrawAddSink(context);
	WIDE_SEP();

	if (ImGui::TreeNodeEx("Grid", treeNodeFlags)) {
		DrawAddGrid(context, false);
		for (Ref<Grid> grid : context->GetSimulation()->Grids()) {
			tid++;
			ImGuiTreeNodeFlags flags = (false ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(tid), flags, "Untitled Grid")) {

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	} else DrawAddGrid(context);
	WIDE_SEP();

	if (ImGui::TreeNodeEx("Particles", treeNodeFlags)) {
		DrawAddParticle(context, false);
		for (Ref<Particle> particle : context->GetSimulation()->Particles()) {
			tid++;
			ImGuiTreeNodeFlags flags = (false ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (ImGui::TreeNodeEx(reinterpret_cast<void*>(tid), flags, "Untitled Particle")) {

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	} else DrawAddParticle(context);
}
