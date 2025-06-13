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
                                               _T("https://github.com/molsonkiko/JsonToolsNppPlugin/releases/");
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

bool NotifyHostedDllEx(SCNotification *notifyCode, bool canShowError)
{
    if ( g_hHostedDll )
    {
        if ( !g_pBeNotifiedFunc )
        {
            g_pBeNotifiedFunc = (BENOTIFIED_FUNC) ::GetProcAddress(g_hHostedDll, "beNotified");
        }

        if ( g_pBeNotifiedFunc )
        {
            g_pBeNotifiedFunc(notifyCode);
            return true;
        }
        else if ( canShowError )
        {
            ShowError(ERR_BENOTIFIED_FUNC_NOT_FOUND);
        }
    }
    return false;
}

bool NotifyHostedDll(unsigned int code, uptr_t idFrom, bool canShowError)
{
    SCNotification scNotific;

    ::ZeroMemory(&scNotific, sizeof(SCNotification));
    scNotific.nmhdr.hwndFrom = g_nppData._nppHandle;
    scNotific.nmhdr.idFrom = idFrom;
    scNotific.nmhdr.code = code;

    return NotifyHostedDllEx(&scNotific, canShowError);
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

    if ( !NotifyHostedDll(NPPN_READY, 0, true) )
        return false;

    if ( !NotifyHostedDll(NPPN_TBMODIFICATION, 0, true) )
        return false;

    if ( g_bDarkModeChangedWhileNotLoaded )
    {
        if ( !NotifyHostedDll(NPPN_DARKMODECHANGED, 0, true) )
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

    // JsonTools relies on NPPN_BUFFERACTIVATED to set its internal state
    uptr_t idFrom = ::SendMessage(g_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
    NotifyHostedDll(NPPN_BUFFERACTIVATED, idFrom, false);

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

void documentation_func()
{
	InvokeHostedPluginFunction(N_DOCUMENTATION);
}

void pretty_json_func()
{
	InvokeHostedPluginFunction(N_PRETTY_JSON);
}

void compress_json_func()
{
	InvokeHostedPluginFunction(N_COMPRESS_JSON);
}

void path_to_pos_func()
{
	InvokeHostedPluginFunction(N_PATH_TO_POS);
}

void select_valid_json_func()
{
	InvokeHostedPluginFunction(N_SELECT_VALID_JSON);
}

void check_json_syntax_func()
{
	InvokeHostedPluginFunction(N_CHECK_JSON_SYNTAX);
}

void json_tree_viewer_func()
{
	InvokeHostedPluginFunction(N_JSON_TREE_VIEWER);
}

void get_json_from_func()
{
	InvokeHostedPluginFunction(N_GET_JSON_FROM);
}

void sort_arrays_func()
{
	InvokeHostedPluginFunction(N_SORT_ARRAYS);
}

void settings_func()
{
	InvokeHostedPluginFunction(N_SETTINGS);
}

void validate_json_func()
{
	InvokeHostedPluginFunction(N_VALIDATE_JSON);
}

void validate_files_func()
{
	InvokeHostedPluginFunction(N_VALIDATE_FILES);
}

void generate_schema_func()
{
	InvokeHostedPluginFunction(N_GENERATE_SCHEMA);
}

void generate_json_func()
{
	InvokeHostedPluginFunction(N_GENERATE_JSON);
}

void run_tests_func()
{
	InvokeHostedPluginFunction(N_RUN_TESTS);
}

void about_func()
{
	InvokeHostedPluginFunction(N_ABOUT);
}

void syntax_errors_func()
{
	InvokeHostedPluginFunction(N_SYNTAX_ERRORS);
}

void json_to_yaml_func()
{
	InvokeHostedPluginFunction(N_JSON_TO_YAML);
}

void parse_json_func()
{
	InvokeHostedPluginFunction(N_PARSE_JSON);
}

void array_to_json_func()
{
	InvokeHostedPluginFunction(N_ARRAY_TO_JSON);
}

void dump_selected_text_func()
{
	InvokeHostedPluginFunction(N_DUMP_SELECTED_TEXT);
}

void dump_json_strings_func()
{
	InvokeHostedPluginFunction(N_DUMP_JSON_STRINGS);
}

void tree_for_ini_func()
{
	InvokeHostedPluginFunction(N_TREE_FOR_INI);
}

void regex_search_func()
{
	InvokeHostedPluginFunction(N_REGEX_SEARCH);
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

    // init shortcuts:
    InitShortcut(N_PRETTY_JSON, true, true, true, 0x50); // Alt+Ctrl+Shift+P
    InitShortcut(N_COMPRESS_JSON, true, true, true, 0x43); // Alt+Ctrl+Shift+C
    InitShortcut(N_PATH_TO_POS, true, true, true, 0x4C); // Alt+Ctrl+Shift+L
    InitShortcut(N_JSON_TREE_VIEWER, true, true, true, 0x4A); // Alt+Ctrl+Shift+J
    InitShortcut(N_GET_JSON_FROM, true, true, true, 0x47); // Alt+Ctrl+Shift+G
    InitShortcut(N_SETTINGS, true, true, true, 0x53); // Alt+Ctrl+Shift+S

    // init menu items:
    InitFuncItem(N_DOCUMENTATION,      _T("&Documentation"), documentation_func, &g_funcShortcut[N_DOCUMENTATION]);
    InitFuncItem(N_PRETTY_JSON,        _T("&Pretty-print current JSON file"), pretty_json_func, &g_funcShortcut[N_PRETTY_JSON]);
    InitFuncItem(N_COMPRESS_JSON,      _T("&Compress current JSON file"), compress_json_func, &g_funcShortcut[N_COMPRESS_JSON]);
    InitFuncItem(N_PATH_TO_POS,        _T("Path to current p&osition"), path_to_pos_func, &g_funcShortcut[N_PATH_TO_POS]);
    InitFuncItem(N_SELECT_VALID_JSON,  _T("Select every val&id JSON in selection"), select_valid_json_func, &g_funcShortcut[N_SELECT_VALID_JSON]);
    InitFuncItem(N_CHECK_JSON_SYNTAX,  _T("Chec&k JSON syntax now"), check_json_syntax_func, &g_funcShortcut[N_CHECK_JSON_SYNTAX]);
    InitFuncItem(N_SEPARATOR_1,        _T(""), NULL,  NULL);
    InitFuncItem(N_JSON_TREE_VIEWER,   _T("Open &JSON tree viewer"), json_tree_viewer_func, &g_funcShortcut[N_JSON_TREE_VIEWER]);
    InitFuncItem(N_GET_JSON_FROM,      _T("&Get JSON from files and APIs"), get_json_from_func, &g_funcShortcut[N_GET_JSON_FROM]);
    InitFuncItem(N_SORT_ARRAYS,        _T("Sort arra&ys"), sort_arrays_func, &g_funcShortcut[N_SORT_ARRAYS]);
    InitFuncItem(N_SETTINGS,           _T("&Settings"), settings_func, &g_funcShortcut[N_SETTINGS]);
    InitFuncItem(N_SEPARATOR_2,        _T(""), NULL,  NULL);
    InitFuncItem(N_VALIDATE_JSON,      _T("&Validate JSON against JSON schema"), validate_json_func, &g_funcShortcut[N_VALIDATE_JSON]);
    InitFuncItem(N_VALIDATE_FILES,     _T("Validate &files with JSON schema if name matches pattern"), validate_files_func, &g_funcShortcut[N_VALIDATE_FILES]);
    InitFuncItem(N_GENERATE_SCHEMA,    _T("Generate sc&hema from JSON"), generate_schema_func, &g_funcShortcut[N_GENERATE_SCHEMA]);
    InitFuncItem(N_GENERATE_JSON,      _T("Generate &random JSON from schema"), generate_json_func, &g_funcShortcut[N_GENERATE_JSON]);
    InitFuncItem(N_SEPARATOR_3,        _T(""), NULL,  NULL);
    InitFuncItem(N_RUN_TESTS,          _T("Run &tests"), run_tests_func, &g_funcShortcut[N_RUN_TESTS]);
    InitFuncItem(N_ABOUT,              _T("A&bout"), about_func, &g_funcShortcut[N_ABOUT]);
    InitFuncItem(N_SYNTAX_ERRORS,      _T("See most recent syntax &errors in this file"), syntax_errors_func, &g_funcShortcut[N_SYNTAX_ERRORS]);
    InitFuncItem(N_JSON_TO_YAML,       _T("JSON to YAML"), json_to_yaml_func, &g_funcShortcut[N_JSON_TO_YAML]);
    InitFuncItem(N_SEPARATOR_4,        _T(""), NULL,  NULL);
    InitFuncItem(N_PARSE_JSON,         _T("Parse JSON Li&nes document"), parse_json_func, &g_funcShortcut[N_PARSE_JSON]);
    InitFuncItem(N_ARRAY_TO_JSON,      _T("&Array to JSON Lines"), array_to_json_func, &g_funcShortcut[N_ARRAY_TO_JSON]);
    InitFuncItem(N_SEPARATOR_5,        _T(""), NULL,  NULL);
    InitFuncItem(N_DUMP_SELECTED_TEXT, _T("D&ump selected text as JSON string(s)"), dump_selected_text_func, &g_funcShortcut[N_DUMP_SELECTED_TEXT]);
    InitFuncItem(N_DUMP_JSON_STRINGS,  _T("Dump JSON string(s) as ra&w text"), dump_json_strings_func, &g_funcShortcut[N_DUMP_JSON_STRINGS]);
    InitFuncItem(N_SEPARATOR_6,        _T(""), NULL,  NULL);
    InitFuncItem(N_TREE_FOR_INI,       _T("Open tree for &INI file"), tree_for_ini_func, &g_funcShortcut[N_TREE_FOR_INI]);
    InitFuncItem(N_SEPARATOR_7,        _T(""), NULL,  NULL);
    InitFuncItem(N_REGEX_SEARCH,       _T("Rege&x search to JSON"), regex_search_func, &g_funcShortcut[N_REGEX_SEARCH]);
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
    if ( notifyCode->nmhdr.hwndFrom == g_nppData._nppHandle ) // Notepad++
    {
        if ( g_hHostedDll )
        {
            // forward everything
            NotifyHostedDllEx(notifyCode, false);
        }
        else // !g_hHostedDll
        {
            switch ( notifyCode->nmhdr.code )
            {
                case NPPN_DARKMODECHANGED:
                    g_bDarkModeChangedWhileNotLoaded = true;
                    break;
            }
        }
    }
    else // Scintilla
    {
        if ( g_hHostedDll )
        {
            switch ( notifyCode->nmhdr.code )
            {
                case SCN_MODIFIED: // forward only this
                    NotifyHostedDllEx(notifyCode, false);
                    break;
            }
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

