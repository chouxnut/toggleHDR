#include <windows.h>
#include <shellapi.h>
#include <vector>

#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc) || !pc) return 0;

    std::vector<DISPLAYCONFIG_PATH_INFO> p(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> m(mc);
    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, p.data(), &mc, m.data(), nullptr)) return 0;

    const auto& t = p[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{
        { DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO, sizeof(g), t.adapterId, t.id }
    };
    if (DisplayConfigGetDeviceInfo(&g.header)) return 0;

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{
        { DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE, sizeof(s), t.adapterId, t.id },
        !g.advancedColorEnabled
    };
    DisplayConfigSetDeviceInfo(&s.header);

    ShellExecuteW(nullptr, L"open", L"ms-settings:display", nullptr, nullptr, SW_SHOWNORMAL);
    Sleep(500);

    STARTUPINFOW si{ sizeof(si), nullptr, nullptr, nullptr, 0,0,0,0,0,0, STARTF_USESHOWWINDOW, SW_HIDE };
    PROCESS_INFORMATION pi{};
    wchar_t cmd[] = L"taskkill /IM SystemSettings.exe /F";

    if (CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 3000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    return 0;
}
