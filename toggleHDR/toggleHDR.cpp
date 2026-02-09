#include <windows.h>
#include <shellapi.h>
#include <vector>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

#define WM_TRAY (WM_USER + 1)
NOTIFYICONDATAW nid{};

void ApplyColorTriggers()
{
    SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
        (LPARAM)L"Color", SMTO_ABORTIFHUNG, 100, nullptr);

    SetDisplayConfig(0, nullptr, 0, nullptr,
        SDC_APPLY | SDC_TOPOLOGY_INTERNAL);

    SetICMMode(nullptr, ICM_ON);
}

void ToggleHDR()
{
    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc)) return;

    std::vector<DISPLAYCONFIG_PATH_INFO> p(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> m(mc);
    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, p.data(), &mc, m.data(), nullptr)) return;

    auto& t = p[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{
        { DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO, sizeof(g), t.adapterId, t.id }
    };

    if (DisplayConfigGetDeviceInfo(&g.header)) return;

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{
        { DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE, sizeof(s), t.adapterId, t.id },
        !g.advancedColorEnabled
    };

    if (!DisplayConfigSetDeviceInfo(&s.header))
        ApplyColorTriggers();
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM)
{
    if (m == WM_TRAY && w == WM_LBUTTONUP)
        ToggleHDR();
    return DefWindowProcW(h, m, w, 0);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"HDRTray";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(wc.lpszClassName, L"", 0,
        0, 0, 0, 0, HWND_MESSAGE, 0, hInst, 0);

    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = LoadIcon(nullptr, IDI_INFORMATION);
    lstrcpyW(nid.szTip, L"HDR Toggle");

    Shell_NotifyIconW(NIM_ADD, &nid);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    Shell_NotifyIconW(NIM_DELETE, &nid);
    return 0;
}
