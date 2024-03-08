#pragma once
#include "Panel.h"
#include <string>

class OverviewPanel : public Panel {
public:
	OverviewPanel();
	virtual void Initialize() override;
	virtual void Update(Editor* context) override;
};
