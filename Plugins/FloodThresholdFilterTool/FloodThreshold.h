#ifndef FloodThreshold_H
#define FloodThreshold_H

//******************************************************************************
// DLL Export definitions
//******************************************************************************
#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined PLUGIN_EXPORT
#  define GIV_PLUGIN_EXPORT __declspec(dllexport)
#elif (defined WIN32 || defined _WIN32 || defined WINCE)
#  define GIV_PLUGIN_EXPORT __declspec(dllimport) // This helps to resolve the problem with plugins link
#else
#  define GIV_PLUGIN_EXPORT
#endif

#endif // FloodThreshold_H
