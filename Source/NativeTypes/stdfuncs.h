#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <cstdlib>
#include <cmath>
#include <limits>

namespace BaluLib
{

	inline int IntSizeOf(int x)
	{
		return ((((x - 1)) / sizeof(int) + 1)*sizeof(int));
	}

	template<class T>
	inline T RadToDeg(T rad)
	{
		return rad*180.0 / M_PI;
	}

	inline float DegToRad(float deg)
	{
		return float(deg / 180.0*M_PI);
	}

	inline double DegToRad(double deg)
	{
		return deg / 180.0*M_PI;
	}

	inline int f2i(float x)//округляет до целого
	{
		const int magic = (150 << 23) | (1 << 22);
		x += *(float*)&magic;
		return *(int*)&x - magic;
	}

	template<class T>
	inline T sqr(T v)
	{
		return v*v;
	}

	inline bool IsPOT(int i)
	{
		assert(i > 0);
		float t = log(float(i)) / log(2.0f);
		return (t - floor(t)) == 0.0f;
	}

	inline int GetGreaterPOT(int i)
	{
		assert(i > 0);
		int p = int(log(float(i)) / log(2.0f));
		return 2 << p;
	}

	template<class T>
	inline T Clamp(T vmin, T vmax, T v)
	{
		return v <= vmax ? (v >= vmin ? v : vmin) : vmax;
	}
	template<class T>
	inline T ClampMax(T vmax, T v)
	{
		return v <= vmax ? v : vmax;
	}

	template<class T>
	inline T ClampMin(T vmin, T v)
	{
		return v >= vmin ? v : vmin;
	}

	template<class T>
	inline T Blend(T vmin, T vmax, T alpha)
	{
		return vmin*(1 - alpha) + vmax*alpha;
	}

	template<class T>
	inline bool IsInMinMax(T v, T v1, T v2)
	{
		return(v >= v1) && (v <= v2);
	}

	template<class T>
	inline bool IsIn(T v, T v1, T v2)
	{
		return((v >= v1) && (v <= v2)) || ((v >= v2) && (v <= v1));
	}

	template<class T>
	inline bool Overlay(T min1, T max1, T min2, T max2)
	{
		return max1 >= min2&&min1 < max2;
	}

	inline int Rand(int i)
	{
		return rand() % (i + 1);
	}

	inline float Randf()
	{
		return rand()*(1.0f / RAND_MAX);
	}

	inline float Randfs()
	{
		return rand()*(2.0f / RAND_MAX) - 1.0f;
	}

	template<class T>
	inline void Swap(T &left, T &right)
	{
		T temp; temp = left; left = right; right = temp;
	}
}