#pragma once
#include "Panel.h"

class HierarchyPanel : public Panel {
public:
	HierarchyPanel();
	virtual void Initialize() override;
	virtual void Update(Editor* context) override;
};
