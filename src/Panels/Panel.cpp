#include "Panel.h"
#include "Core/Editor.h"
#include "imgui.h"

void Panel::CallUpdate(Editor* context) {
    if (m_Enabled) {
	    ImGui::Begin(m_Name.c_str());
        Update(context);
        ImGui::End();
    }
}
