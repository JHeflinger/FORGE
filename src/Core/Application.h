#pragma once
#include <vector>
#include "Forge.h"

class Application {
public:
    Application();
    bool Initialize();
    void Run();
    void Shutdown();
private:
    Forge m_Forge;
};
