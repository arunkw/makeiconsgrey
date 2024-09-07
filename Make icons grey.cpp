#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <psapi.h>
#include <iostream>
#include <vector>
#include <string>

NOTIFYICONDATA nid;

// Example list of applications to monitor
std::vector<std::pair<std::string, std::string>> applications = {
    {"vlc.exe", "C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe"},
    {"mspaint.exe", "C:\\Windows\\System32\\mspaint.exe"},
    // Add other applications here...
};

void AddTrayIcon(HWND hWnd)
{
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1001;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strncpy(nid.szTip, "Taskbar Monitor", sizeof(nid.szTip) - 1);
    nid.szTip[sizeof(nid.szTip) - 1] = '\0';

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_USER + 1:
            if (lParam == WM_RBUTTONDOWN)
            {
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 1001, "Exit");
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1001)
            {
                RemoveTrayIcon();
                PostQuitMessage(0);
            }
            break;

        case WM_DESTROY:
            RemoveTrayIcon();
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Function to check if a process is running
bool IsProcessRunning(const std::string& processName)
{
    DWORD processes[1024], cbNeeded, numProcesses;

    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded))
        return false;

    numProcesses = cbNeeded / sizeof(DWORD);

    for (unsigned int i = 0; i < numProcesses; i++)
    {
        if (processes[i] != 0)
        {
            char processNameBuffer[MAX_PATH] = "<unknown>";
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);

            if (hProcess != NULL)
            {
                HMODULE hMod;
                DWORD cbNeeded2;
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded2))
                {
                    GetModuleBaseNameA(hProcess, hMod, processNameBuffer, sizeof(processNameBuffer));
                }
            }
            CloseHandle(hProcess);

            if (processName.compare(processNameBuffer) == 0)
            {
                return true;  // Process is running
            }
        }
    }

    return false;
}

// Function to unpin from the taskbar
void UnpinFromTaskbar(const std::string& executablePath)
{
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.fMask = SEE_MASK_INVOKEIDLIST;
    sei.lpVerb = "taskbarunpin";
    sei.lpFile = executablePath.c_str();
    sei.nShow = SW_HIDE;

    if (!ShellExecuteEx(&sei))
    {
        std::cerr << "Failed to unpin from taskbar for " << executablePath << std::endl;
    }
}

// Function to pin a shortcut to the taskbar
void PinToTaskbar(const std::string& executablePath)
{
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.fMask = SEE_MASK_INVOKEIDLIST;
    sei.lpVerb = "taskbarpin";
    sei.lpFile = executablePath.c_str();
    sei.nShow = SW_HIDE;

    if (!ShellExecuteEx(&sei))
    {
        std::cerr << "Failed to pin to taskbar for " << executablePath << std::endl;
    }
}

void MonitorTaskbarIcons()
{
    std::string greyIconPath = "C:\\Users\\ADMIN\\Desktop\\Icons\\grey.ico";  // Path to your grey icon

    while (true)
    {
        for (auto& app : applications)
        {
            const std::string& processName = app.first;
            const std::string& executablePath = app.second;

            // Check if the application is running
            if (!IsProcessRunning(processName))
            {
                std::cout << "Application not running: " << processName << std::endl;

                // Unpin from taskbar
                UnpinFromTaskbar(executablePath);

                // Pin with grey icon (same executable path in this case)
                PinToTaskbar(executablePath);
            }
            else
            {
                std::cout << "Application is running: " << processName << std::endl;
            }
        }

        Sleep(5000);  // Check every 5 seconds
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TaskbarMonitor";

    RegisterClassA(&wc);

    HWND hWnd = CreateWindowExA(0, wc.lpszClassName, "Taskbar Monitor", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    AddTrayIcon(hWnd);

    // Start monitoring taskbar icons in a separate thread
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MonitorTaskbarIcons, NULL, 0, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

