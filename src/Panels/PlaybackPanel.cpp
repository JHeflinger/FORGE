#include "PlaybackPanel.h"
#include "Core/Editor.h"
#include "imgui.h"

PlaybackPanel::PlaybackPanel() {
	m_Name = "Simulation Playback";
}

void PlaybackPanel::Update(Editor* context) {
	bool simfinished = context->GetSimulation()->Finished();
	if (!simfinished)
		ImGui::BeginDisabled();
	if (ImGui::Button("Replay Simulation")) {
		context->StartPlayback();
	}
	if (!simfinished)
		ImGui::EndDisabled();
}
