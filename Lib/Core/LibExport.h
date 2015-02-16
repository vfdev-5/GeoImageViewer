#ifndef LIBEXPORT_H
#define LIBEXPORT_H

//******************************************************************************
// DLL Export definitions
//******************************************************************************
#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined LIB_EXPORT
#  define GIV_DLL_EXPORT __declspec(dllexport)
#else
#  define GIV_DLL_EXPORT __declspec(dllimport) // This helps to resolve the problem with Plugin link with AbstractTool plugin interface
#endif

#endif // LIBEXPORT_H
