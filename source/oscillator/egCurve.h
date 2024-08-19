/*
	egCurve.h
	copy from x19850
*/

#ifndef EGCURVE_H_
#define EGCURVE_H_

float egCurveCalc(int curve, float value);

static inline float pedalConv(int min, int max, int curve, int value)
{
	float ret = (float)value / 127.0;	// 0.0~1.0

	if (min == max)
	{	// ==
		ret = min;
	}
	else
	{	// !=
		ret = egCurveCalc(curve, ret);
		ret = ret * (max - min) + min;
	}

	return ret;
}

#endif /* EGCURVE_H_ */
