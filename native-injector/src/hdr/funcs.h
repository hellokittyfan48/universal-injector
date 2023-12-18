#pragma once

#include <ctime>
#include <string>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <filesystem>

void findDLL(std::string& dllName, const std::string& exePath);
bool checkModules(DWORD processId, std::string dllName);
DWORD getProcID(const std::string& processName);
std::string openExeDialog();
std::string exeFromPath(const std::string& fullPath);
void startProc(LPCSTR exePath);
std::string readFile();