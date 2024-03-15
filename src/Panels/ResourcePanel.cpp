#include "ResourcePanel.h"
#include "../Core/Editor.h"
#include "../Utils/ImGuiUtils.h"
#include "imgui.h"

Ref<Grid> GetSelectedGrid(Editor* context) {
	for (Ref<Grid> grid : context->GetSimulation()->Grids())
		if (grid->ID() == context->SelectedID()) return grid;
	return nullptr;
}

Ref<Particle> GetSelectedParticle(Editor* context) {
	for (Ref<Particle> particle : context->GetSimulation()->Particles())
		if (particle->ID() == context->SelectedID()) return particle;
	return nullptr;
}

Ref<Source> GetSelectedSource(Editor* context) {
	for (Ref<Source> source : context->GetSimulation()->Sources())
		if (source->ID() == context->SelectedID()) return source;
	return nullptr;
}

Ref<Sink> GetSelectedSink(Editor* context) {
	for (Ref<Sink> sink : context->GetSimulation()->Sinks())
		if (sink->ID() == context->SelectedID()) return sink;
	return nullptr;
}

void DrawEditName(Ref<Resource> resource) {
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, resource->Name().c_str());
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
		resource->SetName(std::string(buffer));
}

ResourcePanel::ResourcePanel() {
	m_Name = "Resource Properties";
}

void ResourcePanel::Update(Editor* context) {
	Ref<Grid> grid;
	Ref<Sink> sink;
	Ref<Source> source;
	Ref<Particle> particle;

	if ((source = GetSelectedSource(context))) {
		DrawEditName(source);	
	} else if ((sink = GetSelectedSink(context))) {
		DrawEditName(sink);
	} else if ((grid = GetSelectedGrid(context))) {
		DrawEditName(grid);
	} else if ((particle = GetSelectedParticle(context))) {
		DrawEditName(particle);
		ImGui::Dummy({0, 2});
		ImGuiUtils::DrawVec3Control("Position", particle->RawPosition());
	} else {
		ImVec2 winsize = ImGui::GetContentRegionAvail();
		ImGui::SetCursorPos({winsize.x / 2.0f - 60, winsize.y / 2.0f});
		ImGui::Text("No resource selected!");
	}
}
