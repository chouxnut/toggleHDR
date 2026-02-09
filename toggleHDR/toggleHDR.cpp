#include <windows.h>
#include <shellapi.h>
#include <vector>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

BOOL CALLBACK CloseSettingsOnly(HWND h, LPARAM)
{
    wchar_t cls[64], title[256];

    if (!GetClassNameW(h, cls, 64)) return TRUE;
    if (wcscmp(cls, L"ApplicationFrameWindow")) return TRUE;

    GetWindowTextW(h, title, 256);

    // 설정 앱 제목 필터 (한/영)
    if (wcsstr(title, L"설정") || wcsstr(title, L"Settings"))
        PostMessageW(h, WM_CLOSE, 0, 0);

    return TRUE;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc))
        return 0;

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(mc);

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, paths.data(),
                           &mc, modes.data(), nullptr))
        return 0;

    auto& t = paths[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{
        { DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,
          sizeof(g), t.adapterId, t.id }
    };

    if (!DisplayConfigGetDeviceInfo(&g.header))
    {
        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{
            { DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE,
              sizeof(s), t.adapterId, t.id },
            !g.advancedColorEnabled
        };
        DisplayConfigSetDeviceInfo(&s.header);
    }

    // 색상 적용 트리거
    ShellExecuteW(nullptr, L"open", L"ms-settings:display",
                  nullptr, nullptr, SW_SHOWMINNOACTIVE);

    Sleep(600); // 이 값이 핵심 (500~800 안정권)

    EnumWindows(CloseSettingsOnly, 0);
    return 0;
}
