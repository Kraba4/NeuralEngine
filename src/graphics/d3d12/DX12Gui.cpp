#include "DX12RenderEngine.h"

namespace neural::graphics {
void render_imguizmo(ImGuizmo::OPERATION& a_currentGizmoOperation, ImGuizmo::MODE& a_currentGizmoMode,
    Camera& a_camera, DirectX::XMFLOAT4X4* a_selectedMatrix)
{
    ImGuizmo::BeginFrame();
    if (ImGui::Begin("gizmo window"))
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Z))
            a_currentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E))
            a_currentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R))
            a_currentGizmoOperation = ImGuizmo::SCALE;
        if (ImGui::RadioButton("Translate", a_currentGizmoOperation == ImGuizmo::TRANSLATE))
            a_currentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", a_currentGizmoOperation == ImGuizmo::ROTATE))
            a_currentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", a_currentGizmoOperation == ImGuizmo::SCALE))
            a_currentGizmoOperation = ImGuizmo::SCALE;

        if (a_currentGizmoOperation != ImGuizmo::SCALE)
        {
            if (ImGui::RadioButton("Local", a_currentGizmoMode == ImGuizmo::LOCAL))
                a_currentGizmoMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", a_currentGizmoMode == ImGuizmo::WORLD))
                a_currentGizmoMode = ImGuizmo::WORLD;
        }
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(&a_selectedMatrix->_11,
            matrixTranslation, matrixRotation, matrixScale);
        ImGui::InputFloat3("Tr", matrixTranslation);
        ImGui::InputFloat3("Rt", matrixRotation);
        ImGui::InputFloat3("Sc", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation,
            matrixRotation, matrixScale, &a_selectedMatrix->_11);
    }
    ImGui::End();
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(a_camera.getViewPtr(), a_camera.getProjPtr(), a_currentGizmoOperation, a_currentGizmoMode,
        &a_selectedMatrix->_11);
}

void DX12RenderEngine::renderGUI() {
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin("Render settings");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::InputInt("Screenshot Counter", &m_settings.screenshotCounter);
        ImGui::SliderFloat("Rotation angle", &m_settings.rotatingTimeY, 0, DirectX::XM_2PI);
        ImGui::SliderInt("Rotation speed", &m_settings.rotateSpeedY, -10, 10);
        ImGui::Checkbox("Enable rotating", &m_settings.enableRotating);
        ImGui::NewLine();

        ImGui::End();
    }
    render_imguizmo(m_currentGizmoOperation, m_currentGizmoMode, m_settings.camera, m_selectedMatrix);
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
}
}