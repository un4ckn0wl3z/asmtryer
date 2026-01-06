// ProcessUtils.h
#pragma once
#include "Types.h"
#include <vector>

extern std::vector<ProcessInfo> processes;

void RefreshProcessList();
std::string GetProcessArchitecture(DWORD pid);
std::string WCharToString(const wchar_t* wstr);