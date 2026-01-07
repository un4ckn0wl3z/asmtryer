// InjectionLogic.h
#pragma once
#include "Types.h"
#include <vector>
#include <string>
#include <keystone/keystone.h>

extern InjectionData injectionData;
extern bool is32BitMode;
extern std::string currentArch;
extern ks_engine* ksEngine;

bool AssembleToShellcode(const std::string& assembly, std::vector<uint8_t>& shellcode);
bool InjectViaThreadHijacking(DWORD pid, const std::vector<uint8_t>& shellcode);
void RestoreOriginalContext(DWORD pid);
std::string FormatShellcode(const std::vector<uint8_t>& shellcode);
bool ParseHexString(const std::string& input, std::vector<uint8_t>& output);
bool InjectViaCreateRemoteThread(DWORD pid, const std::vector<uint8_t>& shellcode);