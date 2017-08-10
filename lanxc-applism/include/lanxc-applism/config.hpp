

#pragma once


#if defined(LANXC_APPLISM_SHARED_LIBRARY)
  #define LANXC_APPLISM_EXPORT __attribute__((visibility("default")))
  #define LANXC_APPLISM_HIDDEN __attribute__((visibility("hidden")))
#else
  #define LANXC_APPLISM_EXPORT
  #define LANXC_APPLISM_HIDDEN
#endif