#include <windows.h>
#include <shellapi.h>
#include <vector>
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

bool ToggleHDR(){
    UINT32 pc=0,mc=0;
    if(GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS,&pc,&mc)!=ERROR_SUCCESS||pc==0) return false;
    std::vector<DISPLAYCONFIG_PATH_INFO> p(pc);
    std::vector<DISPLAYCONFIG_MODE_INFO> m(mc);
    if(QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,&pc,p.data(),&mc,m.data(),nullptr)!=ERROR_SUCCESS) return false;
    auto& t=p[0].targetInfo;
    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{};
    g.header={DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,sizeof(g),t.adapterId,t.id};
    DisplayConfigGetDeviceInfo(&g.header);
    if(!g.advancedColorSupported) return false;
    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{};
    s.header={DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE,sizeof(s),t.adapterId,t.id};
    s.enableAdvancedColor=!g.advancedColorEnabled;
    return DisplayConfigSetDeviceInfo(&s.header)==ERROR_SUCCESS;
}

void RefreshSettingsApp(){
    ShellExecuteW(nullptr,L"open",L"ms-settings:display",nullptr,nullptr,SW_SHOWNORMAL);
    Sleep(500);
    STARTUPINFOW si{sizeof(si)}; si.dwFlags=STARTF_USESHOWWINDOW; si.wShowWindow=SW_HIDE;
    PROCESS_INFORMATION pi{};
    wchar_t cmd[]=L"taskkill /IM SystemSettings.exe /F";
    CreateProcessW(nullptr,cmd,nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&si,&pi);
    WaitForSingleObject(pi.hProcess,3000);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
}

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
    ToggleHDR();
    RefreshSettingsApp();
    return 0;
}
