#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

static BOOL CALLBACK CloseSettings(HWND h, LPARAM)
{
    wchar_t cls[64];
    GetClassNameW(h, cls, 64);
    if (wcscmp(cls, L"ApplicationFrameWindow"))
        return TRUE;

    DWORD pid;
    GetWindowThreadProcessId(h, &pid);

    HANDLE p = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!p)
        return TRUE;

    wchar_t exe[MAX_PATH];
    DWORD len = MAX_PATH;
    BOOL ok = QueryFullProcessImageNameW(p, 0, exe, &len) &&
              wcsstr(exe, L"SystemSettings.exe");

    CloseHandle(p);

    if (ok) {
        PostMessageW(h, WM_CLOSE, 0, 0);
        return FALSE;
    }
    return TRUE;
}

void ToggleHDR()
{
    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc))
        return;

    auto* paths = new DISPLAYCONFIG_PATH_INFO[pc];
    auto* modes = new DISPLAYCONFIG_MODE_INFO[mc];

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, paths, &mc, modes, nullptr))
        goto done;

    auto& t = paths[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
    g.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
    g.header.size = sizeof(g);
    g.header.adapterId = t.adapterId;
    g.header.id = t.id;

    if (!DisplayConfigGetDeviceInfo(&g.header)) {
        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};
        s.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableAdvancedColor = !g.advancedColorEnabled;
        DisplayConfigSetDeviceInfo(&s.header);

        if (s.enableAdvancedColor) {
            ShellExecuteW(nullptr, L"open", L"ms-settings:display",
                          nullptr, nullptr, SW_SHOWMINNOACTIVE);
            Sleep(0);
            EnumWindows(CloseSettings, 0);
        }
    }

done:
    delete[] paths;
    delete[] modes;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    ToggleHDR();
    return 0;
}
