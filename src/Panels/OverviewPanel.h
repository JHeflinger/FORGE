#pragma once
#include "Panels/Panel.h"

class OverviewPanel : public Panel {
public:
	OverviewPanel();
	virtual void Update(Editor* context) override;
};
