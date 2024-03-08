#include "ViewportPanel.h"
#include "../Renderer/Renderer.h"
#include "../Core/Editor.h"
#include "imgui.h"

ViewportPanel::ViewportPanel() {
	m_Name = "Viewport";
}

void ViewportPanel::Initialize() {
	FramebufferSpecification spec;
	spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
	spec.Width = 1280;
	spec.Height = 720;
	m_Framebuffer = CreateRef<Framebuffer>(spec);
}

void ViewportPanel::CallUpdate(Editor* context) {
	if (m_Enabled) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin(m_Name.c_str(), &m_Enabled);
		Update(context);
		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void ViewportPanel::Update(Editor* context) {
	m_Framebuffer->Bind();

	context->Render();
	context->GetCamera().SetFocused(ImGui::IsWindowHovered() || ImGui::IsWindowFocused());
	m_Framebuffer->ClearAttachment(1, -1);
	if (Resized()) {
		m_Framebuffer->Resize((uint32_t)m_Size.x, (uint32_t)m_Size.y);
		context->GetCamera().SetViewportSize(m_Size.x, m_Size.y);
	}

	m_Framebuffer->Unbind();

	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();
	m_Bounds[0] = { viewportMinRegion.x + viewportOffset.x , viewportMinRegion.y + viewportOffset.y };
	m_Bounds[1] = { viewportMaxRegion.x + viewportOffset.x , viewportMaxRegion.y + viewportOffset.y };
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	m_Size = { viewportPanelSize.x, viewportPanelSize.y };
	uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
	ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_Size.x, m_Size.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
}

bool ViewportPanel::Resized() {
	FramebufferSpecification spec = m_Framebuffer->GetSpecification();
	return m_Size.x > 0.0f && m_Size.y > 0.0f && (spec.Width != m_Size.x || spec.Height != m_Size.y);
}
