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
    // 1️⃣ 먼저 설정창 열기 (아직 화면 정상)
    ShellExecuteW(nullptr, L"open", L"ms-settings:display",
                  nullptr, nullptr, SW_SHOWNORMAL);

    Sleep(300); // 창 생성만 되면 충분

    // 2️⃣ HDR 토글
    UINT32 pc = 0, mc = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pc, &mc))
        return 0;

    DISPLAYCONFIG_PATH_INFO* paths = new DISPLAYCONFIG_PATH_INFO[pc];
    DISPLAYCONFIG_MODE_INFO* modes = new DISPLAYCONFIG_MODE_INFO[mc];

    if (!QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pc, paths, &mc, modes, nullptr))
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

            // 🔴 여기서 화면 OFF
            DisplayConfigSetDeviceInfo(&s.header);
        }
    }

    // 3️⃣ 화면 블랙 타임
    Sleep(1200);

    // 4️⃣ 이 동안 설정 앱 제거
    KillSystemSettingsSilently();

    delete[] paths;
    delete[] modes;
    return 0;
}
