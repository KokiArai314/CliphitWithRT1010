/**
    @file
    @brief

    @author saita
    @date 2009/08/04
    Copyright (C) 2009 Korg Inc. All rights reserved.
*/
#ifndef NURSE_NUMERIC_UTIL_H_
#define NURSE_NUMERIC_UTIL_H_

#include <algorithm>
namespace numericUtility
{
/**
 * 上限、下限の値でクリップします。ポインタ版
 * @note blackfinの時はこっちのほうが重いので使わないほうがいいです。
 * @param x
 * @param min
 * @param max
 */
template<typename Type>
void clip(Type* x, const Type min, const Type max)
{
	*x = (*x < min) ? min : (*x> max) ? max : *x;
}


/**
 * 上限、下限の値でサイクルします。
 * @param x
 * @param min
 * @param max
 */
template<typename Type>
Type cycle(Type x, const Type min, const Type max)
{
	/* clip */
	x -= min;
	x %= (max - min) + 1;
	x += min;
	if (x < min) {
		x = ((max + 1) - (min - x));
	}
	else if (x > max) {
		x = ((min - 1) + (x - max));
	}
	return x;
}

/**
 * 上限、下限の値でクリップします。仮引数版
 * @param x
 * @param min
 * @param max
 * @return
 */
template<typename Type>
Type clip(Type x, const Type& min, const Type& max)
{
	x = (x < min) ? min : x;
	x = (x > max) ? max : x;

	return x;
}
/**
 * 入力値の再マッピングをします。ポインタ版
 * @param x
 * @param fromLow
 * @param fromHigh
 * @param toLow
 * @param toHigh
 * @return
 */
template<typename Type>
void map(Type* x, const Type& fromLow, const Type& fromHigh, const Type& toLow, const Type& toHigh)
{
	 return (*x - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

/**
 * 入力値の再マッピングをします。仮引数版
 * @param x
 * @param fromLow
 * @param fromHigh
 * @param toLow
 * @param toHigh
 * @return
 */
template< typename Type >
Type map(Type x, const Type& fromLow, const Type& fromHigh, const Type& toLow, const Type& toHigh)
{
	 return (x - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

/**
 * 入力値の再マッピングをします。仮引数版 ... ADとかで0~127を0~2みたいにマッピングするとき均等割にする用
 * @param x
 * @param fromLow
 * @param fromHigh
 * @param toLow
 * @param toHigh
 * @return
 */
template<typename Type>
Type mapEvenly(Type x, const Type& fromLow, const Type& fromHigh, const Type& toLow, const Type& toHigh)
{
	if (toLow == toHigh) {
		return toLow;
	}
	if (toLow > toHigh) {
		return toLow - ( ((x - fromLow) * (toLow - toHigh + 1)) / (fromHigh - fromLow + 1) );
	}
	return (x - fromLow) * (toHigh - toLow + 1) / (fromHigh - fromLow + 1) + toLow;
}

/**
 * bitIndexFromLeft番目をbitValue(0 / 1)にセットします。
 * @param value セットしたいやつ
 * @param bitIndexFromLeft 何番目？
 * @param bitValue 1 or 0
 * @return セットしたやつ
 */
template<typename Type>
inline int setBit(const Type value, const Type bitIndexFromLeft, const Type bitValue)
{
    return ((value & ~(1 << bitIndexFromLeft)) | (bitValue << bitIndexFromLeft));
}


/**
 * bitIndexFromLeft番目のビットを返します。
 * @param value 知りたいバイト
 * @param bitIndexFromLeft　何番目？
 * @return 知りたいビット
 */
template<typename Type>
inline int getBit(const Type value, const Type bitIndexFromLeft)
{
	return (value >> bitIndexFromLeft) & 1;
}

/**
 * 符号を返します
 * @param x
 * @return 1/0/-1
 */
template <typename T> inline
int sign(T x) {
    return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}

}

#endif
