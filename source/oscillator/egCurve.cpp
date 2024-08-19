/*
	egCurve.cpp
	copy from x19850
*/

#include "ParamConst.h"
#include "egCurve.h"

float egCurveCalc(int curve, float value)
{
	switch (curve)
	{
	case ParamCurve_Linear:// linear
		break;
	case ParamCurve_Exp1:	// exp1 '=, x^2
		value = value * value;
		break;
	case ParamCurve_Exp2:	// exp2 '=, x^4
		value = value * value;
		value = value * value;
		break;
	case ParamCurve_Exp3:	// exp3 '=, x^8
		value = value * value;
		value = value * value;
		value = value * value;
		break;
	case ParamCurve_Log1:	// log1 '=, 1-(1-x)^2
		value = 1.0f - value;
		value = value * value;
		value = 1.0f - value;
		break;
	case ParamCurve_Log2:	// log2 '=, 1-(1-x)^4
		value = 1.0f - value;
		value = value * value;
		value = value * value;
		value = 1.0f - value;
		break;
	case ParamCurve_Log3:	// log3 '=, 1-(1-x)^8
		value = 1.0f - value;
		value = value * value;
		value = value * value;
		value = value * value;
		value = 1.0f - value;
		break;
	default:
		break;
	}

	return value;
}
