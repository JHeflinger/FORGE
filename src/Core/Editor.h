#pragma once
#include <vector>
#include "Core/Safety.h"
#include "Core/Timestep.h"
#include "Core/Serializer.h"
#include "Simulation/Simulation.h"
#include "Renderer/Camera.h"
#include "Panels/Panel.h"
#include "Events/Event.h"

enum class EditorPrompts {
    NONE,
    NEW,
    SAVE,
    SAVEAS,
    OPEN,
    RUN,
    JOIN
};

enum class EditorPlaybackState {
    NONE,
    READY,
    STARTED,
    PAUSED
};

class Editor {
public:
    Editor();
    bool Initialize();
    void Update(Timestep ts);
    void Shutdown();
	void OnEvent(Event& e);
    void Render();
public:
    std::vector<Ref<Panel>> GetPanels() { return m_Panels; }
    Ref<Simulation> GetSimulation() { return m_Simulation; }
    Ref<Camera> GetCamera() { return m_Camera; }
public:
	uint64_t SelectedID() { return m_SelectedID; }
	void SetSelectedID(uint64_t id) { m_SelectedID = id; }
public:
	void StartPlayback();
	void StopPlayback();
	void PausePlayback();
	void ResumePlayback();
	void StepPlayback(int steps);
    float PlaybackProgression();
    void SetPlaybackSpeed(float speed) { m_PlaybackSpeed = speed; };
    float PlaybackSpeed() { return m_PlaybackSpeed; };
    EditorPlaybackState PlaybackState() { return m_PlaybackState; }
private:
    void InitializePanels();
    void UpdatePanels();
    void DrawMenuBar();
    void ProcessInput();
    void DrawPrompts();
    std::string GetLastSavedString();
private:
    void DrawStaticParticles();
    void DrawPlaybackParticles();
private:
    std::vector<Ref<Panel>> m_Panels;
    Ref<Simulation> m_Simulation;
    Ref<Camera> m_Camera;
	uint64_t m_SelectedID = 0;
    bool m_InputPrimer = true;
    bool m_SimulationSaved = false;
    EditorPrompts m_Prompt = EditorPrompts::NONE;
    float m_LastSavedTime = 0.0f;
	float m_PlaybackFrameTime = 0.0f;
    bool m_PlaybackStarted = false;
    float m_PlaybackSpeed = 1.0f;
    EditorPlaybackState m_PlaybackState = EditorPlaybackState::READY;
};
