#include <windows.h>
#include <shobjidl.h>
#include <vector>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Ole32.lib")

DWORD pid;

BOOL CALLBACK C(HWND h, LPARAM)
{
    DWORD p;
    GetWindowThreadProcessId(h, &p);
    if (p == pid) PostMessageW(h, WM_CLOSE, 0, 0);
    return TRUE;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc)) return 0;

    std::vector<DISPLAYCONFIG_PATH_INFO> p(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> m(mc);
    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, p.data(), &mc, m.data(), nullptr)) return 0;

    auto& t = p[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{
        { DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO, sizeof(g), t.adapterId, t.id }
    };

    if (!DisplayConfigGetDeviceInfo(&g.header)) {
        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{
            { DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE, sizeof(s), t.adapterId, t.id },
            !g.advancedColorEnabled
        };
        DisplayConfigSetDeviceInfo(&s.header);
    }

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    IApplicationActivationManager* a;
    if (SUCCEEDED(CoCreateInstance(
        CLSID_ApplicationActivationManager, nullptr,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&a))))
    {
        a->ActivateApplication(
            L"windows.immersivecontrolpanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel",
            L"page=display", AO_NONE, &pid);
        a->Release();
    }

    for (int i = 0; i < 10; i++) {
        EnumWindows(C, 0);
        Sleep(100);
    }

    CoUninitialize();
    return 0;
}
