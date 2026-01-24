#include <windows.h>
#include <shellapi.h>
#include <vector>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

static BOOL CALLBACK CloseSettings(HWND h, LPARAM)
{
    wchar_t c[64];
    GetClassNameW(h, c, 64);

    if (!wcscmp(c, L"ApplicationFrameWindow")) {
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

    std::vector<DISPLAYCONFIG_PATH_INFO> p(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> m(mc);

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, p.data(), &mc, m.data(), nullptr))
        return;

    auto& t = p[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
    g.header = {
        DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,
        sizeof(g),
        t.adapterId,
        t.id
    };

    if (!DisplayConfigGetDeviceInfo(&g.header)) {
        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};
        s.header = {
            DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE,
            sizeof(s),
            t.adapterId,
            t.id
        };
        s.enableAdvancedColor = !g.advancedColorEnabled;
        DisplayConfigSetDeviceInfo(&s.header);
    }

    ShellExecuteW(nullptr, L"open", L"ms-settings:display", nullptr, nullptr, SW_SHOWMINNOACTIVE);
    Sleep(500);
    EnumWindows(CloseSettings, 0);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    ToggleHDR();
    return 0;
}
