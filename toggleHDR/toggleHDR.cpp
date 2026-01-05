#include <windows.h>
#include <shellapi.h>
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

static HWND target;

BOOL CALLBACK Find(HWND h, LPARAM){
    wchar_t c[64];
    GetClassNameW(h,c,64);
    if(wcscmp(c,L"ApplicationFrameWindow")) return TRUE;

    DWORD p; GetWindowThreadProcessId(h,&p);
    HANDLE P=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,0,p);
    if(!P) return TRUE;

    wchar_t x[MAX_PATH]; DWORD l=MAX_PATH;
    bool ok = QueryFullProcessImageNameW(P,0,x,&l) &&
              wcsstr(x,L"SystemSettings.exe");
    CloseHandle(P);

    if(ok){ target=h; return FALSE; }
    return TRUE;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int){
    UINT32 pc=0,mc=0;
    GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS,&pc,&mc);
    auto p=new DISPLAYCONFIG_PATH_INFO[pc];
    auto m=new DISPLAYCONFIG_MODE_INFO[mc];
    QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,&pc,p,&mc,m,0);
    auto&t=p[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{{DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,sizeof(g),t.adapterId,t.id}};
    DisplayConfigGetDeviceInfo(&g.header);

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{{DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE,sizeof(s),t.adapterId,t.id},!g.advancedColorEnabled};
    DisplayConfigSetDeviceInfo(&s.header);

    if(s.enableAdvancedColor){
        ShellExecuteW(0,L"open",L"ms-settings:display",0,0,SW_SHOWMINNOACTIVE);

        MSG M;
        for(int i=0;i<100 && !target;i++){
            while(PeekMessageW(&M,0,0,0,PM_REMOVE))
                DispatchMessageW(&M);
            EnumWindows(Find,0);
        }

        if(target){
            PostMessageW(target,WM_SYSCOMMAND,SC_CLOSE,0);
            PostMessageW(target,WM_CLOSE,0,0);
        }
    }

    delete[] p;
    delete[] m;
    return 0;
}
