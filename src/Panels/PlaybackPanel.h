#pragma once
#include "Panels/Panel.h"

class PlaybackPanel : public Panel {
public:
	PlaybackPanel();
	virtual void Update(Editor* context) override;
};
