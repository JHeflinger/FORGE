#include "PlanePanel.h"
#include "../Core/Editor.h"
#include "../Renderer/Renderer.h"
#include "imgui.h"

PlanePanel::PlanePanel() {
	m_Name = "Coordinate Plane Properties";
}

void PlanePanel::Initialize() {

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
    ImGui::Checkbox("##show", &m_ShowGrid);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##mirror", &m_Mirror);
    ImGui::Dummy({0, gapsize});
    ImGui::DragFloat("##size", &m_Length, 1.0f, 0.01f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Dummy({0, gapsize});
    ImGui::DragFloat("##stepsize", &m_StepSize, 1.0f, 0.01f, FLT_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##xax", &m_XAxis);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##yax", &m_YAxis);
    ImGui::Dummy({0, gapsize});
    ImGui::Checkbox("##zax", &m_ZAxis);
    ImGui::Dummy({0, gapsize});
    ImGui::Columns(1);
}

void PlanePanel::Render() {
    if (m_ShowGrid) {
        float tracker = 0.0f;
        LineProperties lps = Renderer::GetLineProperties();
        lps.LineColor = { 0.3f, 0.3f, 0.3f, 0.5f };
        lps.LineWidth = 2.0f;
        Renderer::SetLineProperties(lps);
        if (m_XAxis)
            Renderer::DrawLine({{0.0f, 0.0f, 0.0f}, {m_Length, 0.0f, 0.0f}});
        if (m_YAxis)
            Renderer::DrawLine({{0.0f, 0.0f, 0.0f}, {0.0f, m_Length, 0.0f}});
        if (m_ZAxis)
            Renderer::DrawLine({{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, m_Length}});
        int maxgrid = 1000;
        while (tracker <= m_Length && maxgrid > 0) {
            if (m_XAxis) {
                if (m_YAxis) {
                    Renderer::DrawLine({{tracker, m_Mirror ? -1 * m_Length : 0.0f, 0.0f}, {tracker, m_Length, 0.0f}});
                    if (m_Mirror)
                        Renderer::DrawLine({{-1.0f * tracker, -1.0f * m_Length, 0.0f}, {-1.0f * tracker, m_Length, 0.0f}});
                }
                if (m_ZAxis) {
                    Renderer::DrawLine({{tracker, 0.0f,  m_Mirror ? -1 * m_Length : 0.0f}, {tracker, 0.0f, m_Length}});
                    if (m_Mirror)
                        Renderer::DrawLine({{-1.0f * tracker, 0.0f, -1.0f * m_Length}, {-1.0f * tracker, 0.0f, m_Length}});
                }
            }
            if (m_YAxis) {
                if (m_XAxis) {
                    Renderer::DrawLine({{ m_Mirror ? -1 * m_Length : 0.0f, tracker, 0.0f}, {m_Length, tracker, 0.0f}});
                    if (m_Mirror)
                        Renderer::DrawLine({{-1.0f * m_Length, -1.0f * tracker, 0.0f}, {m_Length, -1.0f * tracker, 0.0f}});
                }
                if (m_ZAxis) {
                    Renderer::DrawLine({{0.0f, tracker,  m_Mirror ? -1 * m_Length : 0.0f}, {0.0f, tracker, m_Length}});
                    if (m_Mirror)
                        Renderer::DrawLine({{0.0f, -1.0f * tracker, -1.0f * m_Length}, {0.0f, -1.0f * tracker, m_Length}});
                }
            }
            if (m_ZAxis) {
                if (m_XAxis) {
                    Renderer::DrawLine({{ m_Mirror ? -1 * m_Length : 0.0f, 0.0f, tracker}, {m_Length, 0.0f, tracker}});
                    if (m_Mirror)
                        Renderer::DrawLine({{-1.0f * m_Length, 0.0f, -1.0f * tracker}, {m_Length, 0.0f, -1.0f * tracker}});
                }
                if (m_YAxis) {
                    Renderer::DrawLine({{0.0f,  m_Mirror ? -1 * m_Length : 0.0f, tracker}, { 0.0f, m_Length, tracker}});
                    if (m_Mirror)
                        Renderer::DrawLine({{0.0f, -1.0f * m_Length, -1.0f * tracker}, { 0.0f, m_Length, -1.0f * tracker}});
                }
            }
            tracker += m_StepSize;
            maxgrid--;
        }
    }
}