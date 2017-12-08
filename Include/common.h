#pragma once

#ifdef BALUSCRIPT_DLL_EXPORT
#define BALUSCRIPT_DLL_INTERFACE __declspec(dllexport) 
#else
#define BALUSCRIPT_DLL_INTERFACE __declspec(dllimport)
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