

#pragma once


#if defined(LANXC_UNIXY_SHARED_LIBRARY)
  #define LANXC_UNIXY_EXPORT __attribute__((visibility("default")))
  #define LANXC_UNIXY_HIDDEN __attribute__((visibility("hidden")))
#else
  #define LANXC_UNIXY_EXPORT
  #define LANXC_UNIXY_HIDDEN
#endif