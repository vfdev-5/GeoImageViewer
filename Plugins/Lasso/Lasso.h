#ifndef LASSO_H
#define LASSO_H

//******************************************************************************
// DLL Export definitions
//******************************************************************************
#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined PLUGIN_EXPORT
#  define GIV_PLUGIN_EXPORT __declspec(dllexport)
#else
#  define GIV_PLUGIN_EXPORT __declspec(dllimport)
#endif

#endif // LASSO_H
