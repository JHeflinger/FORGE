#pragma once
#include "Panel.h"

class PlanePanel : public Panel {
public:
	PlanePanel();
	virtual void Initialize() override;
	virtual void Update(Editor* context) override;
	virtual void Render() override;
private:
	bool m_ShowGrid = true;
	bool m_Mirror = true;
	bool m_XAxis = true;
	bool m_YAxis = true;
	bool m_ZAxis = true;
	float m_Length = 10.0f;
	float m_StepSize = 1.0f;
};
