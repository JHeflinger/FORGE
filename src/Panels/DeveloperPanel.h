#pragma once
#include "Panel.h"
#include <utility>
#include <vector>
#include <glm/glm.hpp>

typedef std::pair<glm::vec3, glm::vec3> TestLine; 

struct RendererArtifacts {
	std::vector<TestLine> Lines;
};

class DeveloperPanel : public Panel {
public:
	DeveloperPanel();
	virtual void Initialize() override;
	virtual void Update() override;
private:
	RendererArtifacts m_Artifacts;
};
