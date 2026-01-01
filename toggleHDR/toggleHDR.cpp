#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

static BOOL CALLBACK CloseSettings(HWND h, LPARAM) {
    wchar_t c[64];
    GetClassNameW(h, c, 64);
    if (!wcscmp(c, L"ApplicationFrameWindow")) {
        PostMessageW(h, WM_CLOSE, 0, 0);
        return FALSE;
    }
    return TRUE;
}

int main() {
    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc))
        return 1;

    auto* p = new DISPLAYCONFIG_PATH_INFO[pc];
    auto* m = new DISPLAYCONFIG_MODE_INFO[mc];

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, p, &mc, m, nullptr))
        return 1;

    auto& t = p[0].targetInfo;

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
    }

    ShellExecuteW(nullptr, L"open", L"ms-settings:display", nullptr, nullptr, SW_SHOWMINNOACTIVE);
    Sleep(600);
    EnumWindows(CloseSettings, 0);

    delete[] p;
    delete[] m;
    return 0;
}
