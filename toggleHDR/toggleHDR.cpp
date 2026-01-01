#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <string.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

void KillSystemSettings()
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"SystemSettings.exe") == 0) {
                HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProc) {
                    TerminateProcess(hProc, 0);
                    CloseHandle(hProc);
                }
                break;
            }
        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);
}

int main(int argc, char** argv)
{
    int open_settings = 1;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-icc") == 0)
            open_settings = 0;
    }

    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS)
        return 1;

    auto* paths = new DISPLAYCONFIG_PATH_INFO[pathCount];
    auto* modes = new DISPLAYCONFIG_MODE_INFO[modeCount];

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths, &modeCount, modes, nullptr) != ERROR_SUCCESS) {
        delete[] paths;
        delete[] modes;
        return 1;
    }

    for (UINT32 i = 0; i < pathCount; ++i) {
        const auto& t = paths[i].targetInfo;

        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g = {};
        g.header.type = static_cast<DISPLAYCONFIG_DEVICE_INFO_TYPE>(
            DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO);
        g.header.size = sizeof(g);
        g.header.adapterId = t.adapterId;
        g.header.id = t.id;

        if (DisplayConfigGetDeviceInfo(&g.header) != ERROR_SUCCESS)
            continue;

        const BOOL turnOn = g.advancedColorEnabled ? FALSE : TRUE;

#if defined(DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE)
        DISPLAYCONFIG_SET_HDR_STATE s = {};
        s.header.type = static_cast<DISPLAYCONFIG_DEVICE_INFO_TYPE>(
            DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE);
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableHdr = turnOn ? 1u : 0u;
        DisplayConfigSetDeviceInfo(&s.header);
#else
        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s = {};
        s.header.type = static_cast<DISPLAYCONFIG_DEVICE_INFO_TYPE>(
            DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE);
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableAdvancedColor = turnOn ? 1u : 0u;
        DisplayConfigSetDeviceInfo(&s.header);
#endif

        if (open_settings) {
            STARTUPINFOW si{};
            PROCESS_INFORMATION pi{};
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_SHOWMINNOACTIVE;

            wchar_t cmd[] = L"cmd /c start ms-settings:display";
            CreateProcessW(
                nullptr,
                cmd,
                nullptr,
                nullptr,
                FALSE,
                CREATE_NO_WINDOW,
                nullptr,
                nullptr,
                &si,
                &pi
            );

            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);

            Sleep(1500);
            KillSystemSettings();
        }

        break;
    }

    delete[] paths;
    delete[] modes;
    return 0;
}

#include <windows.h>

int WINAPI WinMain(
    HINSTANCE,
    HINSTANCE,
    LPSTR,
    int
) {
    return main(__argc, __argv);
}
