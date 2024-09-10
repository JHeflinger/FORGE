#pragma once
#include "Panels/Panel.h"

class ControlPanel : public Panel {
public:
	ControlPanel();
	virtual void Initialize() override;
	virtual void Update(Editor* context) override;
};
