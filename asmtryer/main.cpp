// main.cpp
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <tchar.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "DirectXSetup.h"
#include "Style.h"
#include "ProcessUtils.h"
#include "ImGuiWindows.h"
#include "InjectionLogic.h"

#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern DWORD selectedPid;
extern char assemblyBuffer[4096];
extern bool showProcessList;
extern bool showAssemblyEditor;
extern bool showInjectionStatus;
extern bool showAboutModal;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI main(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
                      GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
                      _T("Shellcode Injector"), nullptr };
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, _T("[asmtryer] - Yet Another Shellcode Injector/Thread Hijacking [www.un4ckn0wl3z.dev]"),
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::SetupImGuiHackerStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    RefreshProcessList();

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Windows")) {
                ImGui::MenuItem("Process List", nullptr, &showProcessList);
                ImGui::MenuItem("Assembly Editor", nullptr, &showAssemblyEditor);
                ImGui::MenuItem("Injection Control", nullptr, &showInjectionStatus);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("About")) {
                if (ImGui::MenuItem("About asmtryer")) {
                    showAboutModal = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (showAboutModal) {
            ImGui::OpenPopup("About##modal");
            showAboutModal = false;
        }

        if (ImGui::BeginPopupModal("About##modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "[asmtryer] - Hacker Edition");
            ImGui::Separator();
            ImGui::Text("Version 2.0");
            ImGui::Text("x86 / x64 Thread Hijacking Injector");
            ImGui::Text("Powered by:");
            ImGui::BulletText("Dear ImGui");
            ImGui::BulletText("Keystone Engine");
            ImGui::BulletText("DirectX 11");
            ImGui::Separator();
            ImGui::BulletText("Developer: un4ckn0wl3z");
            ImGui::Separator();
            ImGui::Text("Use responsibly.");
            if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ShowProcessListWindow();
        ShowAssemblyEditorWindow();
        ShowRawShellcodeLoaderWindow();
        ShowInjectionControlWindow();

        ImGui::Render();
        const float clear_color[4] = { 0.00f, 0.02f, 0.00f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    if (ksEngine) ks_close(ksEngine);
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 0;
}