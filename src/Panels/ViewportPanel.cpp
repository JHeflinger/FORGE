#include "ViewportPanel.h"

ViewportPanel::ViewportPanel() {

}

void ViewportPanel::Initialize() {
	FramebufferSpecification spec;
	spec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
	spec.Width = 1280;
	spec.Height = 720;
	m_Framebuffer = CreateRef<Framebuffer>(spec);
}

void ViewportPanel::Update() {
	
}
