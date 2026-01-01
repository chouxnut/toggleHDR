#include <windows.h>
#include <shellapi.h>
#include <string.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR lpCmdLine, int) {
    int open_settings = 1;
    if (lpCmdLine && strstr(lpCmdLine, "--no-icc")) {
        open_settings = 0;
    }

    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS)
        return 1;

    DISPLAYCONFIG_PATH_INFO* paths = new DISPLAYCONFIG_PATH_INFO[pathCount];
    DISPLAYCONFIG_MODE_INFO* modes = new DISPLAYCONFIG_MODE_INFO[modeCount];

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

        BOOL turnOn = g.advancedColorEnabled ? FALSE : TRUE;

#if defined(DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE) && defined(DISPLAYCONFIG_SET_HDR_STATE)
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
            ShellExecuteA(nullptr, "open", "ms-settings:display", nullptr, nullptr, SW_MINIMIZE);
        }

        break;
    }

    delete[] paths;
    delete[] modes;
    return 0;
}
