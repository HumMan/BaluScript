#pragma once

template<class T>
inline bool LexerIsIn(T v, T v1, T v2)
{
	return((v >= v1) && (v <= v2)) || ((v >= v2) && (v <= v1));
}

inline int LexerIntSizeOf(int x)
{
	return ((((x - 1)) / sizeof(int) + 1)*sizeof(int));
}