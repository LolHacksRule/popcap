#include "SexyMath.h"

#include <limits.h>
#include <math.h>
#include <float.h>

#ifndef INFINITY
#define INFINITY HUG_VAL
#endif

#include "SexyMath.inl"

namespace Sexy {

#define MAX_STEP 4096

float fastsinf(float x)
{
	int index = int((x * MAX_STEP) / (pi * 2)) + MAX_STEP;
	return sin_tab[index % MAX_STEP];
}

float fastcosf(float x)
{
	int index = int((x * MAX_STEP) / (pi * 2)) + MAX_STEP;
	return cos_tab[index % MAX_STEP];
}

float fastasinf(float x)
{
	int index = int((x + 1.0f) * (MAX_STEP / 2)) + MAX_STEP;
	return asin_tab[index % MAX_STEP];
}

float fastacosf(float x)
{
	int index = int((x + 1.0f) * (MAX_STEP / 2)) + MAX_STEP;
	return acos_tab[index % MAX_STEP];
}

float fasttanf(float x)
{
	int index = int(((x * MAX_STEP) + (pi / 2)) / pi) + MAX_STEP;
	return tan_tab[index % MAX_STEP];
}

float fastatanf(float x)
{
	int a, b, c;
	float d;

	if (x >= 0.0)
	{
		a = 0;
		b = (MAX_STEP / 2) - 1;
	}
	else
	{
		a = MAX_STEP / 2;
		b = MAX_STEP;
	}

	do
	{
		c = (a + b) >> 1;
		d = x - tan_tab[c];

		if (d > 0)
			a = c + 1;
		else if (d < 0)
			b = c - 1;

	} while ((a <= b) && (d));

	return atan_tab[c];
}

float fastatan2f(float y, float x)
{
#if defined(WIN32) || defined(_WIN32)
#define F_ISNAN(x)    _isnan(x)
#define F_ISFINITE(x) _finite(x)
#define F_ISPZ(x)     (_fpclass(x) == _FPCLASS_PZ)
#define F_ISNZ(x)     (_fpclass(x) == _FPCLASS_NZ)
#define F_ISINF(x)    (_fpclass(x) == _FPCLASS_PINF || _fpclass(x) == _FPCLASS_NINF)
#define F_ISPINF(x)   (_fpclass(x) == _FPCLASS_PINF)
#define F_ISNINF(x)   (_fpclass(x) == _FPCLASS_NINF)
#else
#define F_ISNAN(x)    isnan(x)
#define F_ISFINITE(x) isfinite(x)
#define F_ISPZ(x)     (x == 0.0 && signbit(x) == 0)
#define F_ISNZ(x)     (x == 0.0 && signbit(x) != 0)
#define F_ISINF(x)    fpclassify(x) == FP_INFINITE
#define F_ISPINF(x)   (fpclassify(x) == FP_INFINITE && signbit(x) == 0)
#define F_ISNINF(x)   (fpclassify(x) == FP_INFINITE && signbit(x) != 0)
#endif

	if (F_ISNAN(y))
		return y;

	if (F_ISNAN(x))
		return x;

	if (y == 0)
	{
		if (F_ISNZ(x))
			return F_ISPZ(y) ?  pi : -pi;
		else if (F_ISPZ(x))
			return F_ISPZ(y) ?  +0.0f : -0.0f;
		else if (x < 0)
			return F_ISPZ(y) ?  pi : -pi;
		else if (x > 0)
			return F_ISPZ(y) ?  +0.0f : -0.0f;
	}
	else if (y < 0)
	{
		if (x == 0)
			return -pi / 2;
	}
	else if (y > 0)
	{
		if (x == 0)
			return pi / 2;
	}

	if (y != 0.0f && F_ISFINITE(y) && F_ISINF(x))
	{
		if (F_ISNINF(x))
			return y > 0 ? pi : -pi;
		else
			return y > 0 ? +0.0f : -0.0f;
	}
	else if (F_ISINF(y))
	{
		if (F_ISFINITE(x))
			return F_ISPINF(y) ? pi / 2 : -pi / 2;
		else if (F_ISNINF(x))
			return F_ISPINF(y) ? +3 * pi / 4 : -3 * pi / 4;
		else if (F_ISPINF(x))
			return F_ISPINF(y) ? pi / 4 : -pi / 4;
	}

	return fasttanf(y / x);
}

}
