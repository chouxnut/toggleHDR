#include <windows.h>
#include <shellapi.h>
#include <vector>
#include <tlhelp32.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

DWORD find_settings_pid() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W pe{ sizeof(pe) };
    for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
        if (!_wcsicmp(pe.szExeFile, L"SystemSettings.exe")) {
            CloseHandle(snap);
            return pe.th32ProcessID;
        }
    }

    CloseHandle(snap);
    return 0;
}

void open_and_close_settings() {
    ShellExecuteW(nullptr, L"open", L"ms-settings:display", nullptr, nullptr, SW_SHOWNORMAL);

    constexpr DWORD timeout = 3000;
    constexpr DWORD interval = 20;

    for (DWORD t = 0; t < timeout; t += interval) {
        if (DWORD pid = find_settings_pid()) {
            if (HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid)) {
                TerminateProcess(h, 0);
                CloseHandle(h);
            }
            break;
        }
        Sleep(interval);
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc) != ERROR_SUCCESS || pc == 0)
        return 0;

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(mc);

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, paths.data(), &mc, modes.data(), nullptr) != ERROR_SUCCESS)
        return 0;

    const auto& t = paths[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{
        { DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO, sizeof(g), t.adapterId, t.id }
    };

    if (DisplayConfigGetDeviceInfo(&g.header) != ERROR_SUCCESS || !g.advancedColorSupported)
        return 0;

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{
        { DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE, sizeof(s), t.adapterId, t.id },
        !g.advancedColorEnabled
    };

    DisplayConfigSetDeviceInfo(&s.header);
    open_and_close_settings();
    return 0;
}
