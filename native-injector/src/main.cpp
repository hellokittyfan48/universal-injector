#include <ctime>
#include <locale>
#include <string>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <filesystem>

#include "hdr/funcs.h"

int main(int argc, char* argv[]) {
    SetConsoleTitle("GTA Injector | hellokittyfan48");

    std::string text = R"(
               _____ _______         _____ _   _      _ ______ _____ _______ ____  _____  
              / ____|__   __|/\     |_   _| \ | |    | |  ____/ ____|__   __/ __ \|  __ \ 
             | |  __   | |  /  \      | | |  \| |    | | |__ | |       | | | |  | | |__) |
             | | |_ |  | | / /\ \     | | | . ` |_   | |  __|| |       | | | |  | |  _  / 
             | |__| |  | |/ ____ \   _| |_| |\  | |__| | |___| |____   | | | |__| | | \ \ 
              \_____|  |_/_/    \_\ |_____|_| \_|\____/|______\_____|  |_|  \____/|_|  \_\
                                                                              
                                        github.com/hellokittyfan48
    )";

    std::cout << text << std::endl;

    std::string exePath = readFile();
    const std::string process = exeFromPath(exePath);

    std::string dllName;
    DWORD procId = 0;
    char dllPath[MAX_PATH];
    bool fail = false;
    std::cout << exePath;
    startProc(exePath.c_str());
    findDLL(dllName, argv[0]);
    procId = getProcID(process);

    while (procId == 0) {
        std::cout << exePath;
        std::cout << process << " not found, waiting 2s for it to start...\n";
        procId = getProcID(process);
        clock_t start = clock();
        while (clock() - start < 2000) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    system("cls");
    std::cout << text << std::endl;

    std::cout << "----------------------------------" << std::endl;
    std::cout << "[*] Process: " << process << std::endl;
    std::cout << "[*] PID: " << procId << std::endl;
    std::cout << "[*] DLL: " << dllName << std::endl;
    std::cout << "----------------------------------" << std::endl;

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);

    if (checkModules(procId, dllName)) {
        system("pause");
        return 1;
    }

    if (procId == 0) {
        std::cout << "[-] Failed to find PID" << std::endl;
        fail = true;
    }

    if (!GetFullPathName(dllName.c_str(), MAX_PATH, dllPath, nullptr)) {
        std::cout << "[-] Failed To Get Full Path" << std::endl;
        fail = true;
    }

    if (!std::filesystem::exists(dllPath)) {
        std::cout << "[-] Failed to find DLL" << std::endl;
        fail = true;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

    if (!hProcess) {
        std::cout << "[-] Failed To Open Process" << std::endl;
        fail = true;
    }

    void* allocatedMemory = VirtualAllocEx(hProcess, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (!allocatedMemory) {
        std::cout << "[-] Failed Allocating Memory" << std::endl;
        fail = true;
    }

    if (!WriteProcessMemory(hProcess, allocatedMemory, dllPath, MAX_PATH, nullptr)) {
        std::cout << "[-] Failed Writing to Memory" << std::endl;
        fail = true;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocatedMemory, NULL, nullptr);

    if (!hThread) {
        std::cout << "[-] Failed To Create Remote Thread" << std::endl;
        fail = true;
    }

    CloseHandle(hProcess);
    VirtualFreeEx(hProcess, allocatedMemory, NULL, MEM_RELEASE);

    if (fail) {
        std::cout << "[-] Failed to inject DLL" << std::endl;
    }
    else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        std::cout << "[+] DLL Injected Sucessfully" << std::endl;
    }

    system("pause");

    return 0;
}