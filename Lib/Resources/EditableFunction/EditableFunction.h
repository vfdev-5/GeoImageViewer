//******************************************************************************
// DLL Export definitions
//******************************************************************************
#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined EF_EXPORT_DEF
#  define EF_EXPORT __declspec(dllexport)
#elif (defined WIN32 || defined _WIN32 || defined WINCE)
#  define EF_EXPORT __declspec(dllimport) // This helps to resolve the problem with plugins link
#else
#  define EF_EXPORT
#endif

// ALL INCLUDES
#include <iostream>

extern "C" double EF_EXPORT foo(double value);
