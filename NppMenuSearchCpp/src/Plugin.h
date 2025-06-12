#ifndef _cpp_wrapper_plugin_h_
#define _cpp_wrapper_plugin_h_
//--------------------------------------------------------------------

#define PLUGIN_MENU_NAME       "NppMenuSearch"
#define PLUGIN_INTERNAL_NAME   "NppMenuSearchCpp"
#define HOSTED_PLUGIN_DLL_NAME "NppMenuSearch.dll"

enum eFuncItem : int {
  N_MENU_SEARCH = 0,
  N_CLEAR_RECENTLY_USED,
  N_REPEAT_PREV,
  N_SEPARATOR_1,
  N_ABOUT,
  nbFunc
};

//--------------------------------------------------------------------
#endif
