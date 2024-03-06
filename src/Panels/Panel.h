#pragma once

class Editor;

class Panel {
public:
    virtual ~Panel() = default;
    virtual void Initialize() = 0;
    virtual void Update(Editor* context) = 0;
    virtual void Toggle() { m_Enabled = !m_Enabled; }
protected:
    bool m_Enabled = true;
};
