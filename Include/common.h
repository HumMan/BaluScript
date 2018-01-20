#pragma once

#if defined _WIN32 || defined __CYGWIN__ || defined __MINGW32__
  #ifdef BALUSCRIPT_DLL_EXPORT
    #ifdef __GNUC__
      #define BALUSCRIPT_DLL_INTERFACE __attribute__ ((dllexport))
    #else
      #define BALUSCRIPT_DLL_INTERFACE __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define BALUSCRIPT_DLL_INTERFACE __attribute__ ((dllimport))
    #else
      #define BALUSCRIPT_DLL_INTERFACE __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define BALUSCRIPT_DLL_INTERFACE __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define BALUSCRIPT_DLL_INTERFACE
    #define DLL_LOCAL
  #endif
#endif

template<class T>
inline bool LexerIsIn(T v, T v1, T v2)
{
	return((v >= v1) && (v <= v2)) || ((v >= v2) && (v <= v1));
}

inline int LexerIntSizeOf(int x)
{
	return ((((x - 1)) / sizeof(int) + 1)*sizeof(int));
}
