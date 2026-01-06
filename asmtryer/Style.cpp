// Style.cpp
#include "imgui.h"
#include "Style.h"

void ImGui::SetupImGuiHackerStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;

    colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 0.20f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.05f, 0.00f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.08f, 0.00f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.10f, 0.00f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.00f, 1.00f, 0.00f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.20f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.50f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.70f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.15f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.40f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.15f, 0.00f, 0.80f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.12f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 0.80f, 0.00f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.80f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 0.20f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.00f, 0.35f, 0.00f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.60f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.80f, 0.30f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.70f, 0.20f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.90f, 0.30f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.00f, 1.00f, 0.00f, 0.50f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.80f, 0.20f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 1.00f, 0.30f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.30f, 0.00f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.60f, 0.20f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 1.00f, 0.50f, 1.00f);
}