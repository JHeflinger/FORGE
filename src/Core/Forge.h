#pragma once
#include <vector>
#include "Structures.h"
#include "Safety.h"
#include "../Panels/Panel.h"

class Forge {
public:
    Forge();
    bool Initialize();
    void Run();
    void Shutdown();
private:
    void InitializePanels();
    void UpdatePanels();
private:
    std::vector<Ref<Panel>> m_Panels;
};
