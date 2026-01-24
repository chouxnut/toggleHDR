#include <windows.h>
#include <shellapi.h>
#include <vector>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

DWORD pid;

BOOL CALLBACK CW(HWND h, LPARAM)
{
    DWORD p;
    GetWindowThreadProcessId(h, &p);
    if (p == pid) PostMessageW(h, WM_CLOSE, 0, 0);
    return TRUE;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    UINT32 pc = 0, mc = 0;
    GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc);

    std::vector<DISPLAYCONFIG_PATH_INFO> p(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> m(mc);
    QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, p.data(), &mc, m.data(), 0);

    auto& t = p[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
    g.header = { DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO, sizeof(g), t.adapterId, t.id };
    DisplayConfigGetDeviceInfo(&g.header);

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};
    s.header = { DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE, sizeof(s), t.adapterId, t.id };
    s.enableAdvancedColor = !g.advancedColorEnabled;
    DisplayConfigSetDeviceInfo(&s.header);

    SHELLEXECUTEINFOW e{ sizeof(e), SEE_MASK_NOCLOSEPROCESS };
    e.lpVerb = L"open";
    e.lpFile = L"ms-settings:display";
    e.nShow = SW_SHOWMINNOACTIVE;
    ShellExecuteExW(&e);

    pid = GetProcessId(e.hProcess);
    WaitForInputIdle(e.hProcess, 3000);
    EnumWindows(CW, 0);
    CloseHandle(e.hProcess);
}
