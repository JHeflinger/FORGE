#pragma once
#include "Panel.h"

struct PlanePanelSettings {
	bool ShowGrid = false;
	bool Mirror = true;
	bool XAxis = true;
	bool YAxis = true;
	bool ZAxis = true;
	float Length = 10.0f;
	float StepSize = 1.0f;
};

class PlanePanel : public Panel {
public:
	PlanePanel();
	virtual void Update(Editor* context) override;
	virtual void Render() override;
public:
	PlanePanelSettings& Settings() { return m_Settings; }
private:
	PlanePanelSettings m_Settings;
};
