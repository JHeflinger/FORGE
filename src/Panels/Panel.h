#pragma once
#include <string>

class Editor;

class Panel {
public:
    virtual ~Panel() = default;
    virtual void Initialize() = 0;
    virtual void CallUpdate(Editor* context);
    virtual void Update(Editor* context) = 0;
    virtual void Toggle() { m_Enabled = !m_Enabled; }
    virtual std::string Name() { return m_Name; }
public:
    bool m_Enabled = true;
protected:
    std::string m_Name = "NULL";
};
