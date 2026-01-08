#include <windows.h>
#include <shellapi.h>

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

    auto* p = new DISPLAYCONFIG_PATH_INFO[pc];
    auto* m = new DISPLAYCONFIG_MODE_INFO[mc];

    DISPLAYCONFIG_PATH_TARGET_INFO* target = nullptr;
    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, p, &mc, m, nullptr))
        goto cleanup;

    for (UINT32 i = 0; i < pc; ++i) {
        g.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
        g.header.size = sizeof(g);
        g.header.adapterId = p[i].targetInfo.adapterId;
        g.header.id = p[i].targetInfo.id;

        if (!DisplayConfigGetDeviceInfo(&g.header) &&
            g.advancedColorSupported) {
            target = &p[i].targetInfo;
            break;
        }
    }

    if (!target)
        goto cleanup;

    s.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
    s.header.size = sizeof(s);
    s.header.adapterId = target->adapterId;
    s.header.id = target->id;
    s.enableAdvancedColor = !g.advancedColorEnabled;

    DisplayConfigSetDeviceInfo(&s.header);

    ShellExecuteW(nullptr,
                  L"open",
                  L"ms-settings:display",
                  nullptr,
                  nullptr,
                  SW_SHOWMINNOACTIVE);

    Sleep(0);
    EnumWindows(CloseSettings, 0);

cleanup:
    delete[] p;
    delete[] m;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    ToggleHDR();
    return 0;
}
