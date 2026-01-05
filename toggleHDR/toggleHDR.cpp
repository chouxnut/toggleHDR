#include <windows.h>
#include <dwmapi.h>

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Dwmapi.lib")

bool ToggleHDR()
{
    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount))
        return false;

    auto* paths = new DISPLAYCONFIG_PATH_INFO[pathCount];
    auto* modes = new DISPLAYCONFIG_MODE_INFO[modeCount];

    if (QueryDisplayConfig(
        QDC_ONLY_ACTIVE_PATHS,
        &pathCount, paths,
        &modeCount, modes,
        nullptr))
    {
        delete[] paths;
        delete[] modes;
        return false;
    }

    // 주 디스플레이 기준
    const auto& target = paths[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO get{};
    get.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
    get.header.size = sizeof(get);
    get.header.adapterId = target.adapterId;
    get.header.id = target.id;

    if (DisplayConfigGetDeviceInfo(&get.header) != ERROR_SUCCESS) {
        delete[] paths;
        delete[] modes;
        return false;
    }

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE set{};
    set.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
    set.header.size = sizeof(set);
    set.header.adapterId = target.adapterId;
    set.header.id = target.id;
    set.enableAdvancedColor = !get.advancedColorEnabled;

    if (DisplayConfigSetDeviceInfo(&set.header) != ERROR_SUCCESS) {
        delete[] paths;
        delete[] modes;
        return false;
    }

    // 🔹 즉시 반영 확인
    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO verify = get;
    DisplayConfigGetDeviceInfo(&verify.header);

    if (verify.advancedColorEnabled != set.enableAdvancedColor) {
        // fallback: DWM / 디스플레이 리프레시
        DwmFlush();
        ChangeDisplaySettingsEx(nullptr, nullptr, nullptr, 0, nullptr);
    }

    delete[] paths;
    delete[] modes;
    return true;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    ToggleHDR();
    return 0;
}
