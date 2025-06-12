#ifndef _cpp_wrapper_plugin_h_
#define _cpp_wrapper_plugin_h_
//--------------------------------------------------------------------

#define PLUGIN_MENU_NAME       "Line Filter"
#define PLUGIN_INTERNAL_NAME   "LinefilterCpp"
#define HOSTED_PLUGIN_DLL_NAME "Linefilter3.dll"

enum eFuncItem : int {
  N_LINES_SEL_KEEP = 0,
  N_LINES_SEL_REMOVE,
  N_SEPARATOR_1,
  N_LINES_SEL_KEEP_NO_CASE,
  N_LINES_SEL_REMOVE_NO_CASE,
  N_SEPARATOR_2,
  N_ADVANCED,
  N_SEPARATOR_3,
  N_ABOUT,
  nbFunc
};

//--------------------------------------------------------------------
#endif
