#pragma once
#include "Panel.h"

class ResourcePanel : public Panel {
public:
	ResourcePanel();
	virtual void Update(Editor* context) override;
};
