#include "PlanePanel.h"
#include "../Core/Editor.h"
#include "../Renderer/Renderer.h"
#include "imgui.h"

PlanePanel::PlanePanel() {
	m_Name = "Coordinate Plane Properties";
}

void PlanePanel::Update(Editor* context) {
    float gapsize = 8.0f;
    ImGui::Columns(2);
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Show Coordinate Plane");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Mirror Coordinate Plane");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Coordinate Plane Size");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Step Size");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("X Axis");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Y Axis");
    ImGui::Dummy({0, gapsize});
    ImGui::Text("Z Axis");
    ImGui::Dummy({0, gapsize});
    ImGui::NextColumn();
    gapsize = 2.0f;
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##show", &m_Settings.ShowGrid);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##mirror", &m_Settings.Mirror);
    ImGui::Dummy({0, gapsize});
    ImGui::DragFloat("##size", &m_Settings.Length, 1.0f, 0.01f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Dummy({0, gapsize});
    ImGui::DragFloat("##stepsize", &m_Settings.StepSize, 1.0f, 0.01f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##xax", &m_Settings.XAxis);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##yax", &m_Settings.YAxis);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##zax", &m_Settings.ZAxis);
    ImGui::Dummy({0, gapsize});
    ImGui::Columns(1);
}

void PlanePanel::Render() {
    if (m_Settings.ShowGrid) {
        float tracker = 0.0f;
        LineProperties lps = Renderer::GetLineProperties();
        lps.LineColor = { 0.3f, 0.3f, 0.3f, 0.5f };
        lps.LineWidth = 2.0f;
        Renderer::SetLineProperties(lps);
        if (m_Settings.XAxis)
            Renderer::DrawLine({{0.0f, 0.0f, 0.0f}, {m_Settings.Length, 0.0f, 0.0f}});
        if (m_Settings.YAxis)
            Renderer::DrawLine({{0.0f, 0.0f, 0.0f}, {0.0f, m_Settings.Length, 0.0f}});
        if (m_Settings.ZAxis)
            Renderer::DrawLine({{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, m_Settings.Length}});
        int maxgrid = 1000;
        while (tracker <= m_Settings.Length && maxgrid > 0) {
            if (m_Settings.XAxis) {
                if (m_Settings.YAxis) {
                    Renderer::DrawLine({{tracker, m_Settings.Mirror ? -1 * m_Settings.Length : 0.0f, 0.0f}, {tracker, m_Settings.Length, 0.0f}});
                    if (m_Settings.Mirror)
                        Renderer::DrawLine({{-1.0f * tracker, -1.0f * m_Settings.Length, 0.0f}, {-1.0f * tracker, m_Settings.Length, 0.0f}});
                }
                if (m_Settings.ZAxis) {
                    Renderer::DrawLine({{tracker, 0.0f,  m_Settings.Mirror ? -1 * m_Settings.Length : 0.0f}, {tracker, 0.0f, m_Settings.Length}});
                    if (m_Settings.Mirror)
                        Renderer::DrawLine({{-1.0f * tracker, 0.0f, -1.0f * m_Settings.Length}, {-1.0f * tracker, 0.0f, m_Settings.Length}});
                }
            }
            if (m_Settings.YAxis) {
                if (m_Settings.XAxis) {
                    Renderer::DrawLine({{ m_Settings.Mirror ? -1 * m_Settings.Length : 0.0f, tracker, 0.0f}, {m_Settings.Length, tracker, 0.0f}});
                    if (m_Settings.Mirror)
                        Renderer::DrawLine({{-1.0f * m_Settings.Length, -1.0f * tracker, 0.0f}, {m_Settings.Length, -1.0f * tracker, 0.0f}});
                }
                if (m_Settings.ZAxis) {
                    Renderer::DrawLine({{0.0f, tracker,  m_Settings.Mirror ? -1 * m_Settings.Length : 0.0f}, {0.0f, tracker, m_Settings.Length}});
                    if (m_Settings.Mirror)
                        Renderer::DrawLine({{0.0f, -1.0f * tracker, -1.0f * m_Settings.Length}, {0.0f, -1.0f * tracker, m_Settings.Length}});
                }
            }
            if (m_Settings.ZAxis) {
                if (m_Settings.XAxis) {
                    Renderer::DrawLine({{ m_Settings.Mirror ? -1 * m_Settings.Length : 0.0f, 0.0f, tracker}, {m_Settings.Length, 0.0f, tracker}});
                    if (m_Settings.Mirror)
                        Renderer::DrawLine({{-1.0f * m_Settings.Length, 0.0f, -1.0f * tracker}, {m_Settings.Length, 0.0f, -1.0f * tracker}});
                }
                if (m_Settings.YAxis) {
                    Renderer::DrawLine({{0.0f,  m_Settings.Mirror ? -1 * m_Settings.Length : 0.0f, tracker}, { 0.0f, m_Settings.Length, tracker}});
                    if (m_Settings.Mirror)
                        Renderer::DrawLine({{0.0f, -1.0f * m_Settings.Length, -1.0f * tracker}, { 0.0f, m_Settings.Length, -1.0f * tracker}});
                }
            }
            tracker += m_Settings.StepSize;
            maxgrid--;
        }
    }
}
