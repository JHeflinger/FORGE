#pragma once

class Forge;

class Panel {
public:
    virtual ~Panel() = default;
    virtual void Initialize() = 0;
    virtual void Update(Forge* context) = 0;
    virtual void Toggle() { m_Enabled = !m_Enabled; }
protected:
    bool m_Enabled = true;
};
