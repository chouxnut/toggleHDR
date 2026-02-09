#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

void KillSystemSettingsSilently()
{
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    wchar_t cmd[] = L"taskkill /IM SystemSettings.exe /F";

    if (CreateProcessW(
        nullptr,
        cmd,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi))
    {
        WaitForSingleObject(pi.hProcess, 3000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount))
        return 0;

    DISPLAYCONFIG_PATH_INFO* paths = new DISPLAYCONFIG_PATH_INFO[pathCount];
    DISPLAYCONFIG_MODE_INFO* modes = new DISPLAYCONFIG_MODE_INFO[modeCount];

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths,
                           &modeCount, modes, nullptr))
    {
        delete[] paths;
        delete[] modes;
        return 0;
    }

    const auto& t = paths[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
    g.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
    g.header.size = sizeof(g);
    g.header.adapterId = t.adapterId;
    g.header.id = t.id;

    if (!DisplayConfigGetDeviceInfo(&g.header))
    {
        DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};
        s.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
        s.header.size = sizeof(s);
        s.header.adapterId = t.adapterId;
        s.header.id = t.id;
        s.enableAdvancedColor = !g.advancedColorEnabled;

        // 🔴 이 호출 순간 화면 검게 변함
        DisplayConfigSetDeviceInfo(&s.header);
    }

    // ⏱ 화면 블랙 타임 시작
    Sleep(1500);

    // 색상 적용 트리거
    ShellExecuteW(nullptr, L"open", L"ms-settings:display",
                  nullptr, nullptr, SW_SHOWNORMAL);

    Sleep(800);

    // 설정 앱 제거 (사용자에게 안 보임)
    KillSystemSettingsSilently();

    delete[] paths;
    delete[] modes;
    return 0;
}
