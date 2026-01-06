// ImGuiWindows.cpp
#include "ImGuiWindows.h"
#include "imgui.h"
#include "ProcessUtils.h"
#include "InjectionLogic.h"
#include <string>


DWORD selectedPid = 0;
char assemblyBuffer[4096] = "inc dword ptr [0x18D46898]\nret";
bool showProcessList = true;
bool showAssemblyEditor = true;
bool showInjectionStatus = true;
bool showAboutModal = false;

void ShowProcessListWindow() {
    if (!showProcessList) return;
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Process List", &showProcessList);

    if (ImGui::Button("Refresh")) {
        RefreshProcessList();
    }
    ImGui::SameLine();
    ImGui::Text("Processes: %d", (int)processes.size());

    ImGui::Columns(3, "processcolumns");
    ImGui::Separator();
    ImGui::Text("PID"); ImGui::NextColumn();
    ImGui::Text("Name"); ImGui::NextColumn();
    ImGui::Text("Architecture"); ImGui::NextColumn();
    ImGui::Separator();

    for (const auto& proc : processes) {
        bool isSelected = (selectedPid == proc.pid);
        if (ImGui::Selectable(std::to_string(proc.pid).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
            selectedPid = proc.pid;
            is32BitMode = (proc.arch == "x86");
            currentArch = proc.arch;
        }
        ImGui::NextColumn();
        ImGui::Text("%s", proc.name.c_str()); ImGui::NextColumn();
        ImGui::Text("%s", proc.arch.c_str()); ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::End();
}

void ShowAssemblyEditorWindow() {
    if (!showAssemblyEditor) return;
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
    ImGui::Begin("Assembly Editor", &showAssemblyEditor);

    ImGui::Text("Enter assembly code (%s):", currentArch.c_str());
    ImGui::InputTextMultiline("##assembly", assemblyBuffer, IM_ARRAYSIZE(assemblyBuffer), ImVec2(-1, 150));

    if (ImGui::Button("Assemble to Shellcode")) {
        injectionData.assemblyCode = assemblyBuffer;
        if (AssembleToShellcode(assemblyBuffer, injectionData.shellcode)) {
            ImGui::OpenPopup("Assembly Success");
        }
        else {
            ImGui::OpenPopup("Assembly Failed");
        }
    }

    if (ImGui::BeginPopupModal("Assembly Success", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Successfully assembled %zu bytes", injectionData.shellcode.size());
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Assembly Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to assemble code");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (!injectionData.shellcode.empty()) {
        ImGui::Separator();
        ImGui::Text("Shellcode (%zu bytes):", injectionData.shellcode.size());
        std::string shellcodeStr = FormatShellcode(injectionData.shellcode);
        ImGui::TextWrapped("%s", shellcodeStr.c_str());
    }

    ImGui::End();
}

void ShowInjectionControlWindow() {
    if (!showInjectionStatus) return;
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Injection Control", &showInjectionStatus);

    ImGui::Text("Target PID: %lu (%s)", selectedPid, currentArch.c_str());

    if (selectedPid > 0) {
        if (ImGui::Button("Inject Shellcode")) {
            if (!injectionData.shellcode.empty()) {
                injectionData.targetPid = selectedPid;
                if (InjectViaThreadHijacking(selectedPid, injectionData.shellcode)) {
                    injectionData.injected = true;
                    ImGui::OpenPopup("Injection Success");
                }
                else {
                    ImGui::OpenPopup("Injection Failed");
                }
            }
        }
        if (injectionData.hijacked) {
            ImGui::SameLine();
            if (ImGui::Button("Restore Original RIP/EIP")) {
                RestoreOriginalContext(selectedPid);
                injectionData.hijacked = false;
                ImGui::OpenPopup("Restored");
            }
        }
    }
    else {
        ImGui::Text("Select a process first");
    }

    ImGui::Separator();
    ImGui::Text("Status:");
    if (injectionData.injected)
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Shellcode injected");
    else
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not injected");

    if (injectionData.hijacked)
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Thread hijacked");

    if (ImGui::BeginPopupModal("Injection Success", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Successfully injected shellcode!");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Injection Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to inject shellcode");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Restored", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Original RIP/EIP restored!");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::End();
}