#include "Plugin.h"
#include "npp_files/PluginInterface.h"

#include <windows.h>
#include <tchar.h>


const TCHAR* const PLUGIN_NAME = _T(PLUGIN_MENU_NAME);
const TCHAR* const ERROR_TITLE = _T(PLUGIN_INTERNAL_NAME " - Error");
const TCHAR* const ERR_HOSTED_DLL_NOT_LOADED = _T("Failed to load " HOSTED_PLUGIN_DLL_NAME " !\n") \
                                               _T("Please put " HOSTED_PLUGIN_DLL_NAME " near " PLUGIN_INTERNAL_NAME ".dll !\n") \
                                               _T("\n") \
                                               _T("You can get the latest " HOSTED_PLUGIN_DLL_NAME " here :\n") \
                                               _T("https://www.seelisoft.net/Linefilter3/");
const TCHAR* const ERR_SETINFO_FUNC_NOT_FOUND = _T(HOSTED_PLUGIN_DLL_NAME " does not export a function \"setInfo\"");
const TCHAR* const ERR_GETFUNCSARRAY_FUNC_NOT_FOUND = _T(HOSTED_PLUGIN_DLL_NAME " does not export a function \"getFuncsArray\"");
const TCHAR* const ERR_BENOTIFIED_FUNC_NOT_FOUND = _T(HOSTED_PLUGIN_DLL_NAME " does not export a function \"beNotified\"");
const TCHAR* const ERR_INVOKING_HOSTED_FUNC = _T("Error invoking a function \"%s\" from " HOSTED_PLUGIN_DLL_NAME);

typedef void (*BENOTIFIED_FUNC)(SCNotification *notifyCode); // "beNotified"

HMODULE           g_hThisDll = NULL;
HMODULE           g_hHostedDll = NULL;
BENOTIFIED_FUNC   g_pBeNotifiedFunc = NULL;
bool              g_bDarkModeChangedWhileNotLoaded = false;
NppData           g_nppData;
FuncItem          g_funcItem[nbFunc];
ShortcutKey       g_funcShortcut[nbFunc];
FuncItem*         g_hostedFuncArray = NULL;
int               g_hostedNbFunc = 0;


void ShowError(const TCHAR* szErrorText)
{
    ::MessageBox(g_nppData._nppHandle, szErrorText, ERROR_TITLE, MB_OK | MB_ICONERROR);
}

bool NotifyHostedDll(unsigned int code, bool canShowError)
{
    if ( g_hHostedDll )
    {
        if ( !g_pBeNotifiedFunc )
        {
            g_pBeNotifiedFunc = (BENOTIFIED_FUNC) ::GetProcAddress(g_hHostedDll, "beNotified");
        }

        if ( g_pBeNotifiedFunc )
        {
            SCNotification scNotific;
            ::ZeroMemory(&scNotific, sizeof(SCNotification));
            scNotific.nmhdr.hwndFrom = g_nppData._nppHandle;
            scNotific.nmhdr.code = code;
            g_pBeNotifiedFunc(&scNotific);
            return true;
        }
        else if ( canShowError )
        {
            ShowError(ERR_BENOTIFIED_FUNC_NOT_FOUND);
        }
    }
    return false;
}

bool InitHostedPlugin()
{
    if ( g_hHostedDll )
        return true;

    TCHAR szDllPath[MAX_PATH + 1];
    szDllPath[0] = 0;
    ::GetModuleFileName(g_hThisDll, szDllPath, MAX_PATH);
    TCHAR ch = 0;
    int n = lstrlen(szDllPath) - 1;
    while (n >= 0 && (ch = szDllPath[n]) != _T('\\') && ch != _T('/'))
    {
        n--; 
    }
    szDllPath[n] = 0; // pos of the last '\' or '/'
    lstrcat(szDllPath, _T("\\" HOSTED_PLUGIN_DLL_NAME));

    g_hHostedDll = ::LoadLibrary(szDllPath);
    if ( !g_hHostedDll )
    {
        ShowError(ERR_HOSTED_DLL_NOT_LOADED);
        return false;
    }

    FARPROC pProc = ::GetProcAddress(g_hHostedDll, "setInfo");
    if ( pProc )
    {
        typedef void (*SETINFO_FUNC)(NppData notpadPlusData);
        ((SETINFO_FUNC) pProc)(g_nppData);
    }
    else
    {
        ShowError(ERR_SETINFO_FUNC_NOT_FOUND);
        return false;
    }

    if ( !NotifyHostedDll(NPPN_READY, true) )
        return false;

    if ( !NotifyHostedDll(NPPN_TBMODIFICATION, true) )
        return false;

    if ( g_bDarkModeChangedWhileNotLoaded )
    {
        if ( !NotifyHostedDll(NPPN_DARKMODECHANGED, true) )
            return false;
    }

    pProc = ::GetProcAddress(g_hHostedDll, "getFuncsArray");
    if ( pProc )
    {
        typedef FuncItem* (*GETFUNCSARRAY_FUNC)(int *nbF);
        g_hostedFuncArray = ((GETFUNCSARRAY_FUNC) pProc)(&g_hostedNbFunc);
    }
    else
    {
        ShowError(ERR_GETFUNCSARRAY_FUNC_NOT_FOUND);
        return false;
    }

    return true;
}

void InvokeHostedPluginFunction(eFuncItem nFunc)
{
    if ( InitHostedPlugin() )
    {
        bool isFunctionCalled = false;

        if ( g_hostedFuncArray && nFunc < g_hostedNbFunc )
        {
            PFUNCPLUGINCMD pFunc = g_hostedFuncArray[nFunc]._pFunc;
            if ( pFunc )
            {
                pFunc();
                isFunctionCalled = true;
            }
        }

        if ( !isFunctionCalled )
        {
            TCHAR szError[128];
            ::wsprintf(szError, ERR_INVOKING_HOSTED_FUNC, g_funcItem[nFunc]._itemName);
            ShowError(szError);
        }
    }
}

void lines_sel_keep_func()
{
    InvokeHostedPluginFunction(N_LINES_SEL_KEEP);
}

void lines_sel_remove_func()
{
    InvokeHostedPluginFunction(N_LINES_SEL_REMOVE);
}

void lines_sel_keep_no_case_func()
{
    InvokeHostedPluginFunction(N_LINES_SEL_KEEP_NO_CASE);
}

void lines_sel_remove_no_case_func()
{
    InvokeHostedPluginFunction(N_LINES_SEL_REMOVE_NO_CASE);
}

void advanced_func()
{
    InvokeHostedPluginFunction(N_ADVANCED);
}

void about_func()
{
    InvokeHostedPluginFunction(N_ABOUT);
}

void InitFuncItem(int            nItem,
                  const TCHAR*   szName, 
                  PFUNCPLUGINCMD pFunc, 
                  //bool           bCheck,
                  ShortcutKey*   pShortcut)
{
    lstrcpy(g_funcItem[nItem]._itemName, szName);
    g_funcItem[nItem]._pFunc = pFunc;
    g_funcItem[nItem]._init2Check = false; //bCheck;
    g_funcItem[nItem]._pShKey = pShortcut;
}

void InitShortcut(int nItem, bool isAlt, bool isCtrl, bool isShift, unsigned int key)
{
    g_funcShortcut[nItem]._isAlt = isAlt;
    g_funcShortcut[nItem]._isCtrl = isCtrl;
    g_funcShortcut[nItem]._isShift = isShift;
    g_funcShortcut[nItem]._key = static_cast<unsigned char>(key);
}

void InitializePluginMenu()
{
    // empty shortcuts initialization:
    ::ZeroMemory(g_funcShortcut, nbFunc*sizeof(ShortcutKey));

    // init menu items:
    InitFuncItem(N_LINES_SEL_KEEP,           _T("Keep lines with selection"),                 lines_sel_keep_func,           &g_funcShortcut[N_LINES_SEL_KEEP]);
    InitFuncItem(N_LINES_SEL_REMOVE,         _T("Remove lines with selection"),               lines_sel_remove_func,         &g_funcShortcut[N_LINES_SEL_REMOVE]);
    InitFuncItem(N_SEPARATOR_1,              _T(""),                                          NULL,                          NULL);
    InitFuncItem(N_LINES_SEL_KEEP_NO_CASE,   _T("Keep lines with selection (ignore case)"),   lines_sel_keep_no_case_func,   &g_funcShortcut[N_LINES_SEL_KEEP_NO_CASE]);
    InitFuncItem(N_LINES_SEL_REMOVE_NO_CASE, _T("Remove lines with selection (ignore case)"), lines_sel_remove_no_case_func, &g_funcShortcut[N_LINES_SEL_REMOVE_NO_CASE]);
    InitFuncItem(N_SEPARATOR_2,              _T(""),                                          NULL,                          NULL);
    InitFuncItem(N_ADVANCED,                 _T("Advanced..."),                               advanced_func,                 &g_funcShortcut[N_ADVANCED]);
    InitFuncItem(N_SEPARATOR_3,              _T(""),                                          NULL,                          NULL);
    InitFuncItem(N_ABOUT,                    _T("About..."),                                  about_func,                    &g_funcShortcut[N_ABOUT]);
}

extern "C" BOOL APIENTRY DllMain( 
                       HINSTANCE hInstance, 
                       DWORD     dwReason, 
                       LPVOID    /*lpReserved*/ )
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hThisDll = (HMODULE) hInstance;
            break;

        case DLL_PROCESS_DETACH:
            if ( g_hHostedDll )
            {
                ::FreeLibrary(g_hHostedDll);
                g_hHostedDll = NULL;
                g_pBeNotifiedFunc = NULL;
            }
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        default:
            break;
    }

    return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
    g_nppData = notpadPlusData;
    InitializePluginMenu();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
    return PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
    *nbF = nbFunc;
    return g_funcItem;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
    if ( notifyCode->nmhdr.hwndFrom == g_nppData._nppHandle )
    {
        unsigned int notificationCode = notifyCode->nmhdr.code;
        switch ( notificationCode )
        {
            case NPPN_TBMODIFICATION:
            case NPPN_DARKMODECHANGED:
            case NPPN_READY:
            case NPPN_SHUTDOWN:
                if ( g_hHostedDll )
                {
                    NotifyHostedDll(notificationCode, false);
                }
                else // !g_hHostedDll
                {
                    if (notificationCode == NPPN_DARKMODECHANGED)
                       g_bDarkModeChangedWhileNotLoaded = true;
                }
                break;
        }
    }
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif

