#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <tlhelp32.h>
#include <shlobj.h>
#include <propvarutil.h>
#include <propkey.h>

#include <wincodec.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "windowscodecs.lib")

// Function prototypes
void CreateTrayIcon(HWND hWnd);
void UpdateTaskbarIcons();
bool IsApplicationRunning(const std::wstring& processName);
void ChangeTaskbarIcon(const std::wstring& shortcutPath, bool isGrey);
std::vector<std::wstring> EnumeratePinnedTaskbarShortcuts();
HICON CreateGreyIcon(HICON hIcon);

// Global variables
NOTIFYICONDATA nid = {};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_USER + 1:
            if (lParam == WM_RBUTTONDOWN)
            {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 1, L"Exit");
                SetForegroundWindow(hWnd);
                TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
                DestroyMenu(hMenu);
            }
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) // Exit command
            {
                DestroyWindow(hWnd);
            }
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"TaskbarIconModifier";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(0, CLASS_NAME, L"Taskbar Icon Modifier", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (hWnd == NULL)
    {
        return 0;
    }

    CreateTrayIcon(hWnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        UpdateTaskbarIcons();
    }

    Shell_NotifyIcon(NIM_DELETE, &nid);

    return 0;
}

void CreateTrayIcon(HWND hWnd)
{
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy(nid.szTip, L"Taskbar Icon Modifier");  // Use wcscpy instead of wcscpy_s
    Shell_NotifyIcon(NIM_ADD, &nid);
}

void UpdateTaskbarIcons()
{
    std::vector<std::wstring> pinnedShortcuts = EnumeratePinnedTaskbarShortcuts();
    
    for (const auto& shortcut : pinnedShortcuts)
    {
        std::wstring processName = shortcut.substr(shortcut.find_last_of(L'\\') + 1);
        processName = processName.substr(0, processName.find_last_of(L'.'));
        
        bool isRunning = IsApplicationRunning(processName);
        ChangeTaskbarIcon(shortcut, !isRunning);
    }
}

bool IsApplicationRunning(const std::wstring& processName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W pe32 = { 0 };
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            if (processName == pe32.szExeFile)
            {
                CloseHandle(hSnapshot);
                return true;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return false;
}

void ChangeTaskbarIcon(const std::wstring& shortcutPath, bool isGrey)
{
    IShellLink* pShellLink;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
    
    if (SUCCEEDED(hr))
    {
        IPersistFile* pPersistFile;
        hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
        
        if (SUCCEEDED(hr))
        {
            hr = pPersistFile->Load(shortcutPath.c_str(), STGM_READ);
            
            if (SUCCEEDED(hr))
            {
                wchar_t szIconLocation[MAX_PATH];
                int iIcon;
                hr = pShellLink->GetIconLocation(szIconLocation, MAX_PATH, &iIcon);
                
                if (SUCCEEDED(hr))
                {
                    HICON hIcon = ExtractIcon(NULL, szIconLocation, iIcon);
                    
                    if (hIcon)
                    {
                        if (isGrey)
                        {
                            HICON hGreyIcon = CreateGreyIcon(hIcon);
                            if (hGreyIcon)
                            {
                                pShellLink->SetIconLocation(shortcutPath.c_str(), 0);
                                DestroyIcon(hGreyIcon);
                            }
                        }
                        else
                        {
                            pShellLink->SetIconLocation(szIconLocation, iIcon);
                        }
                        
                        DestroyIcon(hIcon);
                    }
                }
                
                pPersistFile->Save(NULL, TRUE);
            }
            
            pPersistFile->Release();
        }
        
        pShellLink->Release();
    }
}

std::vector<std::wstring> EnumeratePinnedTaskbarShortcuts()
{
    std::vector<std::wstring> shortcuts;
    
    wchar_t szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, szPath)))
    {
        std::wstring folderPath(szPath);
        
        WIN32_FIND_DATAW findFileData;
        HANDLE hFind = FindFirstFileW((folderPath + L"\\*.lnk").c_str(), &findFileData);
        
        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                shortcuts.push_back(folderPath + L"\\" + findFileData.cFileName);
            } while (FindNextFileW(hFind, &findFileData) != 0);
            
            FindClose(hFind);
        }
    }
    
    return shortcuts;
}

HICON CreateGreyIcon(HICON hIcon)
{
    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo))
        return NULL;

    BITMAP bmp;
    if (!GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp))
    {
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        return NULL;
    }

    HDC hdc = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmResult = CreateCompatibleBitmap(hdc, bmp.bmWidth, bmp.bmHeight);
    HGDIOBJ hOldBitmap = SelectObject(hdcMem, hbmResult);

    DrawIconEx(hdcMem, 0, 0, hIcon, bmp.bmWidth, bmp.bmHeight, 0, NULL, DI_NORMAL);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bmp.bmWidth;
    bmi.bmiHeader.biHeight = -bmp.bmHeight;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    std::vector<BYTE> bits(bmp.bmWidth * bmp.bmHeight * 4);
    GetDIBits(hdcMem, hbmResult, 0, bmp.bmHeight, bits.data(), &bmi, DIB_RGB_COLORS);

    for (size_t i = 0; i < bits.size(); i += 4)
    {
        BYTE grey = (BYTE)(0.3 * bits[i + 2] + 0.59 * bits[i + 1] + 0.11 * bits[i]);
        bits[i] = bits[i + 1] = bits[i + 2] = grey;
    }

    SetDIBits(hdcMem, hbmResult, 0, bmp.bmHeight, bits.data(), &bmi, DIB_RGB_COLORS);

    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdc);

    iconInfo.hbmColor = hbmResult;
    HICON hGreyIcon = CreateIconIndirect(&iconInfo);

    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return hGreyIcon;
}

