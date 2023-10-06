#include <ctime>
#include <string>
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <filesystem>

using namespace std;

void findDLL(string& dllName, const string& exePath) {
    string exeDirectory = filesystem::path(exePath).parent_path().string();

    for (const auto& entry : filesystem::directory_iterator(exeDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            dllName = entry.path().filename().string();
            break;
        }
    }
}

bool checkModules(DWORD processId, string dllName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        cerr << "[-] Failed to create snapshot" << endl;
        return false;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(moduleEntry);

    if (Module32First(hSnapshot, &moduleEntry)) {
        do {
            if (moduleEntry.szModule == dllName) {
                cout << "[-] DLL already injected" << endl;
                return true;
            }
        } while (Module32Next(hSnapshot, &moduleEntry));
        return false;
    }

    CloseHandle(hSnapshot);
}

int main(int argc, char* argv[]) {
    SetConsoleTitle("GTA Injector | hellokittyfan47");

    string mark = R"(
               _____ _______         _____ _   _      _ ______ _____ _______ ____  _____  
              / ____|__   __|/\     |_   _| \ | |    | |  ____/ ____|__   __/ __ \|  __ \ 
             | |  __   | |  /  \      | | |  \| |    | | |__ | |       | | | |  | | |__) |
             | | |_ |  | | / /\ \     | | | . ` |_   | |  __|| |       | | | |  | |  _  / 
             | |__| |  | |/ ____ \   _| |_| |\  | |__| | |___| |____   | | | |__| | | \ \ 
              \_____|  |_/_/    \_\ |_____|_| \_|\____/|______\_____|  |_|  \____/|_|  \_\
                                                                              
                                          github.com/hellokittyfan47
    )";

    cout << mark << endl;

    const char* windowTitle = "Grand Theft Auto V";
    string dllName;
    DWORD procId = 0;
    char dllPath[MAX_PATH];
    bool fail = false;

    findDLL(dllName, argv[0]);
    GetWindowThreadProcessId(FindWindow(NULL, windowTitle), &procId);

    if (procId == 0) {
        cout << "GTA not found, waiting 60 seconds for it to start\n";
        clock_t start = clock();
        while (procId == 0) {
            GetWindowThreadProcessId(FindWindow(NULL, windowTitle), &procId);
            if (clock() - start > 60000) {
                break;
            }
            if (procId != 0) {
                system("cls");
                cout << mark << endl;
                break;
            }
        }
    }


    cout << "----------------------------------" << endl;
    cout << "[*] Process: " << windowTitle << endl;
    cout << "[*] PID: " << procId << endl;
    cout << "[*] DLL: " << dllName << endl;
    cout << "----------------------------------" << endl;

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);

    if (checkModules(procId, dllName)) {
        system("pause");
        return 1;
    }

    if (procId == 0) {
        cout << "[-] Failed to find PID" << endl;
        fail = true;
    }

    if (!filesystem::exists(dllName)) {
        cout << "[-] Failed to find DLL" << endl;
        fail = true;
    }

    if (!GetFullPathName(dllName.c_str(), MAX_PATH, dllPath, nullptr)) {
        cout << "[-] Failed To Get Full Path" << endl;
        fail = true;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

    if (!hProcess) {
        cout << "[-] Failed To Open Process" << endl;
        fail = true;
    }

    void* allocatedMemory = VirtualAllocEx(hProcess, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (!allocatedMemory) {
        cout << "[-] Failed Allocating Memory" << endl;
        fail = true;
    }

    if (!WriteProcessMemory(hProcess, allocatedMemory, dllPath, MAX_PATH, nullptr)) {
        cout << "[-] Failed Writing to Memory" << endl;
        fail = true;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocatedMemory, NULL, nullptr);

    if (!hThread) {
        cout << "[-] Failed To Create Remote Thread" << endl;
        fail = true;
    }

    CloseHandle(hProcess);
    VirtualFreeEx(hProcess, allocatedMemory, NULL, MEM_RELEASE);

    if (fail) {
        cout << "[-] Failed to inject DLL" << endl;
    }
    else {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        cout << "[+] DLL Injected Sucessfully" << endl;
    }

    system("pause");

    return 0;
}