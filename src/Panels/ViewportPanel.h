#pragma once
#include "Panel.h"
#include "../Core/Safety.h"
#include "../Renderer/Framebuffer.h"
#include "glm/glm.hpp"

class ViewportPanel : public Panel {
public:
	ViewportPanel();
	virtual void Initialize() override;
	virtual void CallUpdate(Editor* context) override;
	virtual void Update(Editor* context) override;
	bool Resized();
private:
	Ref<Framebuffer> m_Framebuffer;
	glm::vec2 m_Size = {0, 0};
	glm::vec2 m_Bounds[2];
};
