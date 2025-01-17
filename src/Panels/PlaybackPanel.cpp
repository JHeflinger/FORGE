#include "PlaybackPanel.h"
#include "Core/Editor.h"
#include "Core/Log.h"
#include "imgui.h"

PlaybackPanel::PlaybackPanel() {
	m_Name = "Simulation Playback";
}

void PlaybackPanel::Update(Editor* context) {
	static int stepsize = 1;
    float gapsize = 8.0f;
	bool simfinished = context->GetSimulation()->Finished();
	if (!simfinished)
		ImGui::BeginDisabled();
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 160);
    ImGui::Columns(2);
    ImGui::Text("Simulation Playback");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Speed");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Step");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Stepsize");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Progress");
	ImGui::NextColumn();
    gapsize = 2.0f;
	switch (context->PlaybackState()) {
		case EditorPlaybackState::READY:
			if (ImGui::Button("Play", {75, 0})) {
				context->StartPlayback();
			}
			break;
		case EditorPlaybackState::STARTED:
			if (ImGui::Button("Pause", {75, 0})) {
				context->PausePlayback();
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop", {75, 0})) {
				context->StopPlayback();
			}
			break;
		case EditorPlaybackState::PAUSED:
			if (ImGui::Button("Resume", {75, 0})) {
				context->ResumePlayback();
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop", {75, 0})) {
				context->StopPlayback();
			}
			break;
		default: FATAL("Unhandled playback state detected");
	}
    ImGui::Dummy({0, gapsize});
	float pbspeed = context->PlaybackSpeed();
	if (ImGui::DragFloat("##simspeed", &pbspeed, 0.01f, 0.001f, 100.0f, "%.3f"))
		context->SetPlaybackSpeed(pbspeed);
    ImGui::Dummy({0, gapsize});
	if (simfinished && context->PlaybackState() != EditorPlaybackState::PAUSED)
		ImGui::BeginDisabled();
	if (ImGui::Button("Forward", {75, 0})) {
		context->StepPlayback(stepsize);
	}
	ImGui::SameLine();
	if (ImGui::Button("Back", {75, 0})) {
		context->StepPlayback(-stepsize);
	}
	if (simfinished && context->PlaybackState() != EditorPlaybackState::PAUSED)
		ImGui::EndDisabled();
    ImGui::Dummy({0, gapsize});
	ImGui::DragInt("##stepsize", &stepsize, 1, 1, INT_MAX);
    ImGui::Dummy({0, gapsize});
	float progress = 100.0f * context->PlaybackProgression();
	char buffer[1024];
	snprintf(buffer, 1023, "%.3f%%%% complete", progress);
    ImGui::Text("%s", buffer);
	ImGui::Columns(1);
	if (!simfinished)
		ImGui::EndDisabled();
}
