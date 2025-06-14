#ifndef _cpp_wrapper_plugin_h_
#define _cpp_wrapper_plugin_h_
//--------------------------------------------------------------------

#define PLUGIN_MENU_NAME       "JsonTools"
#define PLUGIN_INTERNAL_NAME   "JsonToolsCpp"
#define HOSTED_PLUGIN_DLL_NAME "JsonTools.dll"

enum eFuncItem : int {
  N_DOCUMENTATION = 0,
  N_PRETTY_JSON,
  N_COMPRESS_JSON,
  N_PATH_TO_POS,
  N_SELECT_VALID_JSON,
  N_CHECK_JSON_SYNTAX,
  N_SEPARATOR_1,

  N_JSON_TREE_VIEWER,
  N_GET_JSON_FROM,
  N_SORT_ARRAYS,
  N_SETTINGS,
  N_SEPARATOR_2,

  N_VALIDATE_JSON,
  N_VALIDATE_FILES,
  N_GENERATE_SCHEMA,
  N_GENERATE_JSON,
  N_SEPARATOR_3,

  N_RUN_TESTS,
  N_ABOUT,
  N_SYNTAX_ERRORS,
  N_JSON_TO_YAML,
  N_SEPARATOR_4,

  N_PARSE_JSON,
  N_ARRAY_TO_JSON,
  N_SEPARATOR_5,

  N_DUMP_SELECTED_TEXT,
  N_DUMP_JSON_STRINGS,
  N_SEPARATOR_6,

  N_TREE_FOR_INI,
  N_SEPARATOR_7,

  N_REGEX_SEARCH,

  nbFunc
};

//--------------------------------------------------------------------
#endif
