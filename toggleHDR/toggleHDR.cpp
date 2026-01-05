#include <windows.h>
#include <shellapi.h>
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Shell32.lib")

static HWINEVENTHOOK h; static bool d;

void CALLBACK W(HWINEVENTHOOK, DWORD e, HWND w, LONG, LONG, DWORD, DWORD){
    wchar_t c[32]; DWORD p,l=MAX_PATH; wchar_t x[MAX_PATH];
    if(e!=EVENT_OBJECT_CREATE||d||!w||
       !GetClassNameW(w,c,32)||wcscmp(c,L"ApplicationFrameWindow")||
       !GetWindowThreadProcessId(w,&p)) return;
    HANDLE P=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,0,p);
    if(P&&QueryFullProcessImageNameW(P,0,x,&l)&&wcsstr(x,L"SystemSettings.exe")){
        PostMessageW(w,WM_CLOSE,0,0); d=1; UnhookWinEvent(h);
    }
    if(P) CloseHandle(P);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int){
    UINT32 pc=0,mc=0; GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS,&pc,&mc);
    auto p=new DISPLAYCONFIG_PATH_INFO[pc];
    auto m=new DISPLAYCONFIG_MODE_INFO[mc];
    QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS,&pc,p,&mc,m,0);
    auto&t=p[0].targetInfo;

    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO g{{DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,sizeof(g),t.adapterId,t.id}};
    DisplayConfigGetDeviceInfo(&g.header);

    DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE s{{DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE,sizeof(s),t.adapterId,t.id},!g.advancedColorEnabled};
    DisplayConfigSetDeviceInfo(&s.header);

    if(s.enableAdvancedColor){
        d=0;
        h=SetWinEventHook(EVENT_OBJECT_CREATE,EVENT_OBJECT_CREATE,0,W,0,0,
            WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
        ShellExecuteW(0,L"open",L"ms-settings:display",0,0,SW_SHOWMINNOACTIVE);
        MSG M; while(!d&&GetMessageW(&M,0,0,0)) DispatchMessageW(&M);
    }
    delete[] p; delete[] m; return 0;
}
