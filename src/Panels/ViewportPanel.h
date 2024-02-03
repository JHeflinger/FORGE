
#pragma once
#include "Panel.h"
#include "../Core/Safety.h"
#include "../Core/Structures.h"
#include "../Renderer/Framebuffer.h"

class ViewportPanel : public Panel {
public:
	ViewportPanel();
    virtual void Initialize() override;
    virtual void Update() override;
private:
	Ref<Framebuffer> m_Framebuffer;
	Dimensions m_Size = {0, 0};
	Dimensions m_Bounds[2];
};
