#include "hdr/funcs.h"

void findDLL(std::string& dllName, const std::string& exePath) {
    std::string exeDirectory = std::filesystem::path(exePath).parent_path().string();

    for (const auto& entry : std::filesystem::directory_iterator(exeDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".dll") {
            dllName = entry.path().filename().string();
            break;
        }
    }
}

bool checkModules(DWORD processId, std::string dllName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[-] Failed to create snapshot" << std::endl;
        return false;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(moduleEntry);

    if (Module32First(hSnapshot, &moduleEntry)) {
        do {
            if (moduleEntry.szModule == dllName) {
                std::cout << "[-] DLL already injected" << std::endl;
                return true;
            }
        } while (Module32Next(hSnapshot, &moduleEntry));
        return false;
    }

    CloseHandle(hSnapshot);
    return false;
}

DWORD getProcID(const std::string& processName) {
    HANDLE snapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapShot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (::Process32First(snapShot, &entry)) {
        do {
            // Convert wide string to narrow string
            std::string exeFileName(entry.szExeFile, entry.szExeFile + strlen(entry.szExeFile));

            if (processName == exeFileName) {
                ::CloseHandle(snapShot);
                return entry.th32ProcessID;
            }
        } while (::Process32Next(snapShot, &entry));
    }

    ::CloseHandle(snapShot);
    return 0;
}

std::string openExeDialog() {
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = "All Files (*.*)\0*.exe\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = "Select an Executable";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        return fileName;
    }
    else {
        exit(0);
    }
}

void startProc(LPCSTR exePath) {
    PROCESS_INFORMATION pi = (PROCESS_INFORMATION)malloc(sizeof(PROCESS_INFORMATION));
    STARTUPINFO si = { sizeof(si) };

    if (CreateProcess(
        exePath,         // path to exe
        nullptr,         // command line
        nullptr,         // process security attributes
        nullptr,         // thread security attributes
        FALSE,           // inherit handles from parent process
        0,               // flags
        nullptr,         // environment block
        nullptr,         // starting directory
        &si,             // startup info
        &pi              // process info
    )) {
        //WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

std::string readFile() {
    const std::string filename = "path.txt";
    std::ifstream inputFile(filename);
    std::string line;

    if (!std::filesystem::exists(filename)) {
        std::ofstream outputFile(filename);
        std::string path = openExeDialog();
        outputFile << path;
        outputFile.close();
        MessageBox(NULL, "Please restart the injector (this will appear only once)\nThis is here until I fix wtv the fuck is happening", "Restart", NULL);
        exit(0);
        return path;
    }

    if (inputFile.is_open()) {
        if (std::getline(inputFile, line)) {
            inputFile.close();
            return line;
        }
        inputFile.close();
    }
}