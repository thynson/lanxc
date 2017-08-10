

#pragma once


#if defined(LANXC_CORE_SHARED_LIBRARY)
  #if defined(BUILD_LANXC_CORE_SHARED_LIBRARY)
    #ifdef _WIN32
      #define LANXC_CORE_EXPORT __declspec(dllexport)
      #define LANXC_CORE_HIDDEN
    #elif defined(unix) || defined(__unix__) || defined(__APPLE__)
      #define LANXC_CORE_EXPORT __attribute__((visibility("default")))
      #define LANXC_CORE_HIDDEN  __attribute__((visibility("hidden")))
    #else
      #define LANXC_CORE_EXPORT
      #define LANXC_CORE_HIDDEN
    #endif
  #else
    #ifdef _WIN32
      #define LANXC_CORE_EXPORT __declspec(dllimport)
      #define LANXC_CORE_HIDDEN
    #elif defined(unix) || defined(__unix__) || defined(__APPLE__)
      #define LANXC_CORE_EXPORT
      #define LANXC_CORE_HIDDEN
    #else
      #define LANXC_CORE_EXPORT
      #define LANXC_CORE_HIDDEN
    #endif
  #endif
#else
  #define LANXC_CORE_EXPORT
  #define LANXC_CORE_HIDDEN
#endif