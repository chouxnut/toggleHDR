#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <shobjidl.h>
#include <propkey.h>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

HWINEVENTHOOK hook;

void CALLBACK E(HWINEVENTHOOK, DWORD e, HWND h, LONG, LONG, DWORD, DWORD)
{
    if (e != EVENT_OBJECT_CREATE || !IsWindow(h)) return;

    wchar_t c[32];
    if (!GetClassNameW(h, c, 32) || wcscmp(c, L"ApplicationFrameWindow")) return;

    IPropertyStore* s;
    if (SUCCEEDED(SHGetPropertyStoreForWindow(h, IID_PPV_ARGS(&s)))) {
        PROPVARIANT v;
        PropVariantInit(&v);
        if (SUCCEEDED(s->GetValue(PKEY_AppUserModel_ID, &v)) &&
            v.vt == VT_LPWSTR &&
            !wcscmp(v.pwszVal, L"windows.immersivecontrolpanel_cw5n1h2txyewy"))
            PostMessageW(h, WM_CLOSE, 0, 0);
        PropVariantClear(&v);
        s->Release();
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    hook = SetWinEventHook(
        EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE,
        nullptr, E, 0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    UINT32 pc = 0, mc = 0;
    GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc);

    std::vector<DISPLAYCONFIG_PATH_INFO> p(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> m(mc);
    QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, p.data(), &mc, m.data(), nullptr);

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

    ShellExecuteW(nullptr, L"open", L"ms-settings:display", nullptr, nullptr, SW_SHOWMINNOACTIVE);

    Sleep(2000);
    UnhookWinEvent(hook);
}
