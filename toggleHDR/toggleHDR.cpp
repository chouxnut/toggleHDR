#include <windows.h>
#include <vector>

#pragma comment(lib,"User32.lib")

static void ApplyColorTriggers()
{
    SendMessageTimeoutW(
        HWND_BROADCAST,
        WM_SETTINGCHANGE,
        0,
        (LPARAM)L"Color",
        SMTO_ABORTIFHUNG,
        100,
        nullptr
    );

    SetDisplayConfig(
        0, nullptr,
        0, nullptr,
        SDC_APPLY | SDC_TOPOLOGY_INTERNAL
    );

    SetICMMode(nullptr, ICM_ON);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount))
        return 0;

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    if (QueryDisplayConfig(
        QDC_ONLY_ACTIVE_PATHS,
        &pathCount, paths.data(),
        &modeCount, modes.data(),
        nullptr))
        return 0;

    for (auto& path : paths)
    {
        auto& t = path.targetInfo;

        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
        g.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
        g.header.size = sizeof(g);
        g.header.adapterId = t.adapterId;
        g.header.id = t.id;

        if (DisplayConfigGetDeviceInfo(&g.header))
            continue;

        if (!g.advancedColorSupported)
            continue;

        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};
        s.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableAdvancedColor = !g.advancedColorEnabled;

        if (!DisplayConfigSetDeviceInfo(&s.header))
            ApplyColorTriggers();

        break; // 첫 HDR 지원 디스플레이만
    }

    return 0;
}
