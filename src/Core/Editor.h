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
    RUN
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
	uint64_t m_PlaybackFrame = 0;
    bool m_PlaybackStarted = false;
};
