#include "Application.h"
#include "Core/Log.h"
#include "Core/Serializer.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Renderer/Renderer.h"
#include "Events/Input.h"
#include "Utils/FileUtils.h"
#include "Utils/DialogUtils.h"

Application::Application() {
	m_Editor = CreateRef<Editor>();
}

bool Application::Initialize() {
	Log::Init();
	if (FileUtils::Exists("settings.fconf"))
		Serializer::DeserializeEditor(m_Editor, FileUtils::Read("settings.fconf"));
	return true;
}

void Application::Run() {
	// Create window 
	WindowProperties props;
	props.Name = "FORGE";
	props.Width = 1600;
	props.Height = 900;
	props.Fullscreen = false;
	m_Window = CreateRef<Window>(props);
	m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	
	// Create GUI 
	m_GUI = CreateRef<GUI>(m_Window);
	m_GUI->Initialize();

	// Initialize Input handler
	Input::Initialize(this);

	// Initialize file dialog system
	DialogUtils::Initialize(this);

	// Initialize Forge
	m_Editor->Initialize();
	
	while (m_Running) {
		float time = (float)glfwGetTime();
		Timestep timestep = time - m_FrameTime;
		m_FrameTime = time;
		m_GUI->Begin();
		Update(timestep);
		m_GUI->End();
		m_Window->Update();
	}
}

void Application::Update(Timestep ts) {
	static bool dockspace_open = true;
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen) {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else {
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;

	if (!opt_padding) ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockspace_open, window_flags);
	if (!opt_padding) ImGui::PopStyleVar();

	if (opt_fullscreen) ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowMinSize.x = 380.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	style.WindowMinSize.x = 32.0f;

	m_Editor->Update(ts);

	ImGui::End();
}

void Application::Shutdown() {
	m_Editor->Shutdown();
	m_GUI->Shutdown();
}

void Application::OnEvent(Event& e) {
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClosed));
	dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));
	m_GUI->OnEvent(e);
	m_Editor->OnEvent(e);
}

bool Application::OnWindowClosed(WindowCloseEvent& e) {
	INFO("Saving editor...");
	FileUtils::Write(Serializer::SerializeEditor(m_Editor), "settings.fconf");
	m_Running = false;
	return true;
}

bool Application::OnWindowResize(WindowResizeEvent& e) {
	if (e.GetWidth() == 0 || e.GetHeight() == 0) {
		m_Minimized = true;
		return false;
	}
	m_Minimized = false;
	Renderer::SetViewport({0, 0, e.GetWidth(), e.GetHeight()});
	return false;
}
