#include <windows.h>
#include <shellapi.h>
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

void KillSystemSettingsSilently()
{
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    wchar_t cmd[] = L"taskkill /IM SystemSettings.exe /F";

    if (CreateProcessW(
        nullptr, cmd,
        nullptr, nullptr, FALSE,
        CREATE_NO_WINDOW,
        nullptr, nullptr,
        &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, 3000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // --------------------------
    // 1️⃣ HDR 토글
    // --------------------------
    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount))
        return 0;

    DISPLAYCONFIG_PATH_INFO* paths = new DISPLAYCONFIG_PATH_INFO[pathCount];
    DISPLAYCONFIG_MODE_INFO* modes = new DISPLAYCONFIG_MODE_INFO[modeCount];

    if (!QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths, &modeCount, modes, nullptr))
    {
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

            // 🔴 이 순간 모니터가 꺼짐 (HDR 토글)
            DisplayConfigSetDeviceInfo(&s.header);
        }
    }

    // --------------------------
    // 2️⃣ 화면 블랙 타임
    // --------------------------
    Sleep(800); // 블랙 타임 확보 (환경에 따라 700~1200ms 조절)

    // --------------------------
    // 3️⃣ 설정창 열기 + 바로 닫기
    // --------------------------
    ShellExecuteW(nullptr, L"open", L"ms-settings:display",
                  nullptr, nullptr, SW_SHOWNORMAL);
    Sleep(300); // 창이 열리기만 하면 충분

    KillSystemSettingsSilently(); // 설정창 제거

    delete[] paths;
    delete[] modes;
    return 0;
}
