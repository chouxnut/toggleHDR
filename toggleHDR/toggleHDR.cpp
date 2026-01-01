#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <string.h>

void KillSystemSettings()
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(snap, &pe)) {
        do {
            if (!_wcsicmp(pe.szExeFile, L"SystemSettings.exe")) {
                HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (h) {
                    TerminateProcess(h, 0);
                    CloseHandle(h);
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
    for (int i = 1; i < argc; ++i)
        if (!strcmp(argv[i], "--no-icc"))
            open_settings = 0;

    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc))
        return 1;

    auto* paths = new DISPLAYCONFIG_PATH_INFO[pc];
    auto* modes = new DISPLAYCONFIG_MODE_INFO[mc];

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, paths, &mc, modes, nullptr)) {
        delete[] paths;
        delete[] modes;
        return 1;
    }

    for (UINT32 i = 0; i < pc; ++i) {
        auto& t = paths[i].targetInfo;

        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
        g.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
        g.header.size = sizeof(g);
        g.header.adapterId = t.adapterId;
        g.header.id = t.id;

        if (DisplayConfigGetDeviceInfo(&g.header))
            continue;

        BOOL on = g.advancedColorEnabled ? FALSE : TRUE;

#if defined(DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE)
        DISPLAYCONFIG_SET_HDR_STATE s{};
        s.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE;
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableHdr = on;
        DisplayConfigSetDeviceInfo(&s.header);
#else
        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};
        s.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableAdvancedColor = on;
        DisplayConfigSetDeviceInfo(&s.header);
#endif

        if (open_settings) {
            ShellExecuteW(nullptr, L"open", L"ms-settings:display", nullptr, nullptr, SW_SHOWMINNOACTIVE);
            Sleep(1500);
            KillSystemSettings();
        }
        break;
    }

    delete[] paths;
    delete[] modes;
    return 0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return main(__argc, __argv);
}
