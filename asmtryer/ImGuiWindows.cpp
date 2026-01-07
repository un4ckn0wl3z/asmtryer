// ImGuiWindows.cpp
#include "ImGuiWindows.h"
#include "imgui.h"
#include "ProcessUtils.h"
#include "InjectionLogic.h"
#include <string>

bool showRawShellcodeLoader = true;
ShellcodeSource currentShellcodeSource = ShellcodeSource::None;

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

void ShowRawShellcodeLoaderWindow() {
    if (!showRawShellcodeLoader) return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Raw Shellcode Loader", &showRawShellcodeLoader);

    static char rawInputBuffer[16384] = "";  // Larger buffer for big payloads

    ImGui::Text("Paste raw shellcode here:");
    ImGui::TextWrapped("Supported formats: \\x90\\xcc\\xeb, 90 cc eb 1e, 9090cceb1e, etc.");

    // Resizable input area
    float footerHeight = ImGui::GetFrameHeightWithSpacing() * 3 + ImGui::GetStyle().ItemSpacing.y * 2;

    ImGui::InputTextMultiline(
        "##rawinput",
        rawInputBuffer,
        IM_ARRAYSIZE(rawInputBuffer),
        ImVec2(-1, -footerHeight),
        ImGuiInputTextFlags_AllowTabInput
    );

    if (ImGui::Button("Load Raw Shellcode", ImVec2(150, 0))) {
        std::vector<uint8_t> parsed;
        if (ParseHexString(rawInputBuffer, parsed)) {
            injectionData.shellcode = std::move(parsed);
            currentShellcodeSource = ShellcodeSource::RawInput;  // Mark source
            ImGui::OpenPopup("Raw Load Success");
        }
        else {
            ImGui::OpenPopup("Raw Load Failed");
        }
    }

    ImGui::SameLine();
    ImGui::Text("Current loaded: %zu bytes", injectionData.shellcode.size());

    if (ImGui::Button("Clear Shellcode")) {
        injectionData.shellcode.clear();
        currentShellcodeSource = ShellcodeSource::None;
        rawInputBuffer[0] = '\0';  // Optional: clear input buffer
    }

    // Success popup
    if (ImGui::BeginPopupModal("Raw Load Success", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Successfully loaded %zu bytes!", injectionData.shellcode.size());
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // Failure popup
    if (ImGui::BeginPopupModal("Raw Load Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to parse hex string.");
        ImGui::TextWrapped("Make sure it's valid hex with optional \\x, spaces, or none.");
        if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::End();
}


void ShowAssemblyEditorWindow() {
    if (!showAssemblyEditor) return;

    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Assembly Editor", &showAssemblyEditor);

    ImGui::Text("Enter assembly code (%s):", currentArch.c_str());

    float footerHeight = ImGui::GetStyle().ItemSpacing.y * 2 +
        ImGui::GetFrameHeightWithSpacing() * 3;

    ImGui::InputTextMultiline(
        "##assembly",
        assemblyBuffer,
        IM_ARRAYSIZE(assemblyBuffer),
        ImVec2(-1, -footerHeight - ImGui::GetTextLineHeightWithSpacing()),
        ImGuiInputTextFlags_AllowTabInput
    );

    if (ImGui::Button("Assemble to Shellcode")) {
        injectionData.assemblyCode = assemblyBuffer;
        if (AssembleToShellcode(assemblyBuffer, injectionData.shellcode)) {
            currentShellcodeSource = ShellcodeSource::Assembler;  // Mark source
            ImGui::OpenPopup("Assembly Success");
        }
        else {
            ImGui::OpenPopup("Assembly Failed");
        }
    }

    // Popups
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

    // Shared shellcode preview (used by both assembly and raw loader)
    if (!injectionData.shellcode.empty()) {
        ImGui::Separator();
        ImGui::Text("Current Shellcode (%zu bytes):", injectionData.shellcode.size());

        std::string shellcodeStr = FormatShellcode(injectionData.shellcode);
        ImGui::TextWrapped("%s", shellcodeStr.c_str());

        ImGui::SameLine();
        std::string clipboardText;
        for (unsigned char byte : injectionData.shellcode) {
            char buf[16];
            snprintf(buf, sizeof(buf), "\\x%02X", byte);
            clipboardText += buf;
        }

        if (ImGui::Button("Copy to Clipboard")) {
            ImGui::SetClipboardText(clipboardText.c_str());
            ImGui::OpenPopup("Copied!");
        }

        if (ImGui::BeginPopupModal("Copied!", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Shellcode copied as hex string!");
            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}
void ShowInjectionControlWindow() {
    if (!showInjectionStatus) return;

    ImGui::SetNextWindowSize(ImVec2(450, 250), ImGuiCond_FirstUseEver);
    ImGui::Begin("Injection Control", &showInjectionStatus);

    ImGui::Text("Target PID: %lu (%s)", selectedPid, currentArch.c_str());

    // Show shellcode source and size
    if (!injectionData.shellcode.empty()) {
        ImGui::Separator();
        ImGui::Text("Loaded Shellcode: %zu bytes", injectionData.shellcode.size());

        // Display source with color
        switch (currentShellcodeSource) {
        case ShellcodeSource::Assembler:
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Source: Assembled from Assembly Editor");
            break;
        case ShellcodeSource::RawInput:
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Source: Loaded from Raw Shellcode");
            break;
        default:
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "Source: Unknown");
            break;
        }
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "No shellcode loaded");
    }

    ImGui::Separator();

    if (selectedPid > 0) {
        bool canInject = !injectionData.shellcode.empty();

        if (!canInject) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        }

        if (ImGui::Button("Inject Shellcode", ImVec2(200, 40))) {
            if (canInject) {
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

        if (!canInject) ImGui::PopStyleColor(3);

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
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Select a process first");
    }

    ImGui::Separator();
    ImGui::Text("Status:");
    if (injectionData.injected)
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Shellcode injected");
    else
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not injected");

    if (injectionData.hijacked)
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Thread hijacked");

    // Popups remain the same
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