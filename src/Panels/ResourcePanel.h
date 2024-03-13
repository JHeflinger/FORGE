#pragma once
#include "Panel.h"

class ResourcePanel : public Panel {
public:
	ResourcePanel();
	virtual void Initialize() override;
	virtual void Update(Editor* context) override;
};
