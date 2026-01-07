// InjectionLogic.cpp
#include "InjectionLogic.h"
#include <windows.h>
#include <tlhelp32.h>
#include <sstream>
#include <iomanip>

InjectionData injectionData;
bool is32BitMode = false;
std::string currentArch = "x64";
ks_engine* ksEngine = nullptr;

std::string FormatShellcode(const std::vector<uint8_t>& shellcode) {
    std::stringstream ss;
    for (size_t i = 0; i < shellcode.size(); ++i) {
        if (i > 0) ss << " ";
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(shellcode[i]);
        if ((i + 1) % 16 == 0) ss << "\n";
    }
    return ss.str();
}

bool AssembleToShellcode(const std::string& assembly, std::vector<uint8_t>& shellcode) {
    ks_mode mode = is32BitMode ? KS_MODE_32 : KS_MODE_64;

    if (ksEngine) {
        ks_close(ksEngine);
        ksEngine = nullptr;
    }

    ks_err err = ks_open(KS_ARCH_X86, mode, &ksEngine);
    if (err != KS_ERR_OK) return false;

    unsigned char* encode = nullptr;
    size_t size = 0, count = 0;
    if (ks_asm(ksEngine, assembly.c_str(), 0, &encode, &size, &count) != KS_ERR_OK) {
        ks_free(encode);
        return false;
    }

    shellcode.assign(encode, encode + size);
    ks_free(encode);
    return true;
}

bool InjectViaThreadHijacking(DWORD pid, const std::vector<uint8_t>& shellcode) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;

    LPVOID remoteMem = VirtualAllocEx(hProcess, nullptr, shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteMem) { CloseHandle(hProcess); return false; }

    if (!WriteProcessMemory(hProcess, remoteMem, shellcode.data(), shellcode.size(), nullptr)) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    THREADENTRY32 te{ sizeof(te) };
    HANDLE hThread = nullptr;
    if (Thread32First(hSnap, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                if (hThread) break;
            }
        } while (Thread32Next(hSnap, &te));
    }
    CloseHandle(hSnap);

    if (!hThread) {
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    SuspendThread(hThread);
    bool success = false;

    if (is32BitMode) {
        WOW64_CONTEXT ctx{ sizeof(ctx) };
        ctx.ContextFlags = CONTEXT_CONTROL;
        if (Wow64GetThreadContext(hThread, &ctx)) {
            injectionData.originalRip = ctx.Eip;
            ctx.Eip = (DWORD)(uintptr_t)remoteMem;
            if (Wow64SetThreadContext(hThread, &ctx)) success = true;
        }
    }
    else {
        CONTEXT ctx{ sizeof(ctx) };
        ctx.ContextFlags = CONTEXT_CONTROL;
        if (GetThreadContext(hThread, &ctx)) {
            injectionData.originalRip = ctx.Rip;
            ctx.Rip = (DWORD64)(uintptr_t)remoteMem;
            if (SetThreadContext(hThread, &ctx)) success = true;
        }
    }

    ResumeThread(hThread);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    if (success) {
        injectionData.hijacked = true;
        return true;
    }

    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
    return false;
}

void RestoreOriginalContext(DWORD pid) {
    if (!injectionData.hijacked) return;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnap == INVALID_HANDLE_VALUE) { CloseHandle(hProcess); return; }

    THREADENTRY32 te{ sizeof(te) };
    HANDLE hThread = nullptr;
    if (Thread32First(hSnap, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) {
                hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                if (hThread) break;
            }
        } while (Thread32Next(hSnap, &te));
    }
    CloseHandle(hSnap);
    if (!hThread) { CloseHandle(hProcess); return; }

    SuspendThread(hThread);

    if (is32BitMode) {
        WOW64_CONTEXT ctx{ sizeof(ctx) };
        ctx.ContextFlags = CONTEXT_CONTROL;
        if (Wow64GetThreadContext(hThread, &ctx)) {
            ctx.Eip = (DWORD)injectionData.originalRip;
            Wow64SetThreadContext(hThread, &ctx);
        }
    }
    else {
        CONTEXT ctx{ sizeof(ctx) };
        ctx.ContextFlags = CONTEXT_CONTROL;
        if (GetThreadContext(hThread, &ctx)) {
            ctx.Rip = injectionData.originalRip;
            SetThreadContext(hThread, &ctx);
        }
    }

    ResumeThread(hThread);
    CloseHandle(hThread);
    CloseHandle(hProcess);
}

// Parses common hex formats: \x90\xCC, 90 cc eb, 9090CCeb1e, etc.
bool ParseHexString(const std::string& input, std::vector<uint8_t>& output) {
    output.clear();
    std::string cleaned;

    for (char c : input) {
        if (isxdigit(c)) {
            cleaned += toupper(c);
        }
    }

    if (cleaned.empty()) return false;

    // Handle \x escapes if present in original
    std::string hex;
    size_t i = 0;
    std::string orig = input;
    while (i < orig.length()) {
        if (orig[i] == '\\' && i + 1 < orig.length() && orig[i + 1] == 'x') {
            if (i + 3 < orig.length() && isxdigit(orig[i + 2]) && isxdigit(orig[i + 3])) {
                hex += toupper(orig[i + 2]);
                hex += toupper(orig[i + 3]);
                i += 4;
            }
            else {
                i++;
            }
        }
        else if (isxdigit(orig[i])) {
            hex += toupper(orig[i]);
            i++;
        }
        else {
            i++;
        }
    }

    if (hex.length() % 2 != 0) return false;

    for (size_t j = 0; j < hex.length(); j += 2) {
        std::string byteStr = hex.substr(j, 2);
        uint8_t byte = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
        output.push_back(byte);
    }

    return !output.empty();
}