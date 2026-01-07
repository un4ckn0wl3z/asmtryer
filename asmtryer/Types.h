// Types.h
#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <cstdint>

struct ProcessInfo {
    DWORD pid;
    std::string name;
    std::string arch;
};

struct InjectionData {
    std::string assemblyCode;
    std::vector<uint8_t> shellcode;
    DWORD targetPid = 0;
    uintptr_t originalRip = 0;
    bool injected = false;
    bool hijacked = false;
};

enum class ShellcodeSource {
    None,
    Assembler,
    RawInput
};

enum class InjectionMethod {
    ThreadHijacking,
    CreateRemoteThread
};