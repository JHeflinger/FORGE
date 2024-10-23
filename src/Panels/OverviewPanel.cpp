#include "OverviewPanel.h"
#include "Core/Editor.h"
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

void OverviewPanel::Update(Editor* context) {
	const ImGuiTreeNodeFlags treeNodeFlags = 
		ImGuiTreeNodeFlags_SpanAvailWidth | 
		ImGuiTreeNodeFlags_AllowItemOverlap | 
		ImGuiTreeNodeFlags_FramePadding | 
		ImGuiTreeNodeFlags_DefaultOpen;

	if (ImGui::TreeNodeEx("Sources", treeNodeFlags)) {
		DrawAddSource(context, false);
		for (Ref<Source> source : context->GetSimulation()->Sources()) {
			ImGuiTreeNodeFlags flags = (source->ID() == context->SelectedID() ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(source->ID()), flags, source->Name().c_str());
			if (ImGui::IsItemClicked())
				context->SetSelectedID(source->ID());
			if (opened) ImGui::TreePop();
		}
		ImGui::TreePop();
	} else DrawAddSource(context);
	WIDE_SEP();

	if (ImGui::TreeNodeEx("Sinks", treeNodeFlags)) {
		DrawAddSink(context, false);
		for (Ref<Sink> sink : context->GetSimulation()->Sinks()) {
			ImGuiTreeNodeFlags flags = (sink->ID() == context->SelectedID()  ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(sink->ID()), flags, sink->Name().c_str());
			if (ImGui::IsItemClicked())
				context->SetSelectedID(sink->ID());
			if (opened) ImGui::TreePop();
		}
		ImGui::TreePop();
	} else DrawAddSink(context);
	WIDE_SEP();

	if (ImGui::TreeNodeEx("Grid", treeNodeFlags)) {
		DrawAddGrid(context, false);
		for (Ref<Grid> grid : context->GetSimulation()->Grids()) {
			ImGuiTreeNodeFlags flags = (grid->ID() == context->SelectedID()  ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(grid->ID()), flags, grid->Name().c_str());
			if (ImGui::IsItemClicked()) 
				context->SetSelectedID(grid->ID());
			if (opened) ImGui::TreePop();
		}
		ImGui::TreePop();
	} else DrawAddGrid(context);
	WIDE_SEP();

	if (ImGui::TreeNodeEx("Particles", treeNodeFlags)) {
		DrawAddParticle(context, false);
		uint64_t particle_to_remove = 0;
		bool remove_particle = false;
		for (Ref<Particle> particle : context->GetSimulation()->Particles()) {
			ImGuiTreeNodeFlags flags = (particle->ID() == context->SelectedID()  ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(particle->ID()), flags, particle->Name().c_str());
			if (ImGui::IsItemClicked())
				context->SetSelectedID(particle->ID());
			char buff[1024];
			sprintf(buff, "##particle_number_%llu", (long long unsigned int)particle->ID());
			if (ImGui::BeginPopupContextItem(buff)) {
				if (ImGui::MenuItem("Delete")) {
					remove_particle = true;
					particle_to_remove = particle->ID();
				}
            	ImGui::EndPopup();
        	}
			if (opened) ImGui::TreePop();
		}
		if (remove_particle) {
			for (size_t i = 0; i < context->GetSimulation()->Particles().size(); i++) {
				if (context->GetSimulation()->Particles()[i]->ID() == particle_to_remove) {
					context->GetSimulation()->Particles().erase(context->GetSimulation()->Particles().begin() + i);
					break;
				}
			}
		}
		ImGui::TreePop();
	} else DrawAddParticle(context);
}
