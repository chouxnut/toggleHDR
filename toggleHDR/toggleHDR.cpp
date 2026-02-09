#include <windows.h>
#include <shellapi.h>
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
    UINT32 pc=0,mc=0;
    if(GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS,&pc,&mc)) return 0;
    DISPLAYCONFIG_PATH_INFO* p=new DISPLAYCONFIG_PATH_INFO[pc];
    DISPLAYCONFIG_MODE_INFO* m=new DISPLAYCONFIG_MODE_INFO[mc];

    if(!QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,&pc,p,&mc,m,nullptr))
    {
        auto& t=p[0].targetInfo;
        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
        g.header={DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,sizeof(g),t.adapterId,t.id};
        if(!DisplayConfigGetDeviceInfo(&g.header))
        {
            DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};
            s.header={DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE,sizeof(s),t.adapterId,t.id};
            s.enableAdvancedColor=!g.advancedColorEnabled;
            DisplayConfigSetDeviceInfo(&s.header);
        }
    }

    ShellExecuteW(nullptr,L"open",L"ms-settings:display",nullptr,nullptr,SW_SHOWNORMAL);
    Sleep(180);

    STARTUPINFOW si{}; PROCESS_INFORMATION pi{};
    si.cb=sizeof(si); si.dwFlags=STARTF_USESHOWWINDOW; si.wShowWindow=SW_HIDE;
    wchar_t cmd[]=L"taskkill /IM SystemSettings.exe /F";
    if(CreateProcessW(nullptr,cmd,nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&si,&pi))
        WaitForSingleObject(pi.hProcess,3000),CloseHandle(pi.hProcess),CloseHandle(pi.hThread);

    delete[] p; delete[] m;
    return 0;
}
