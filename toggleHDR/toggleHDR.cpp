#include <windows.h>
#include <shellapi.h>
#include <string.h>

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
        g.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
        g.header.size = sizeof(g);
        g.header.adapterId = t.adapterId;
        g.header.id = t.id;

        if (DisplayConfigGetDeviceInfo(&g.header) != ERROR_SUCCESS)
            continue;

        BOOL turnOn = g.advancedColorEnabled ? FALSE : TRUE;

#if defined(DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE)
        DISPLAYCONFIG_SET_HDR_STATE s = {};
        s.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_HDR_STATE;
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableHdr = turnOn ? 1u : 0u;
        DisplayConfigSetDeviceInfo(&s.header);
#else
        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s = {};
        s.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableAdvancedColor = turnOn ? 1u : 0u;
        DisplayConfigSetDeviceInfo(&s.header);
#endif

        if (open_settings) {
            ShellExecuteW(
                nullptr,
                L"open",
                L"ms-settings:display",
                nullptr,
                nullptr,
                SW_SHOWNOACTIVATE
            );
        }

        break;
    }

    delete[] paths;
    delete[] modes;
    return 0;
}

int WINAPI WinMain(
    HINSTANCE,
    HINSTANCE,
    LPSTR,
    int
) {
    return main(__argc, __argv);
}
