#include "SexyMath.h"

#include <limits.h>
#include <math.h>

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
	int index = int(floor(x * MAX_STEP / pi + 0.5)) + MAX_STEP;
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
	return fasttanf(y / x);
}

}
