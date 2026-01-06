// ProcessUtils.cpp
#include "ProcessUtils.h"
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <string>

std::vector<ProcessInfo> processes;

std::string WCharToString(const wchar_t* wstr) {
    if (!wstr) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) return {};
    std::string str(size_needed - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size_needed, nullptr, nullptr);
    return str;
}

std::string GetProcessArchitecture(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return "Unknown";

    BOOL isWow64 = FALSE;
    if (IsWow64Process(hProcess, &isWow64)) {
        CloseHandle(hProcess);
        return isWow64 ? "x86" : "x64";
    }
    CloseHandle(hProcess);
    return "Unknown";
}

void RefreshProcessList() {
    processes.clear();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe{ sizeof(pe) };
    if (Process32First(snapshot, &pe)) {
        do {
            if (pe.th32ProcessID == 0) continue;

            ProcessInfo info;
            info.pid = pe.th32ProcessID;
            info.name = WCharToString(pe.szExeFile);
            info.arch = GetProcessArchitecture(pe.th32ProcessID);

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
            if (hProcess) {
                processes.push_back(info);
                CloseHandle(hProcess);
            }
        } while (Process32Next(snapshot, &pe));
    }
    CloseHandle(snapshot);
}