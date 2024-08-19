/*
 * version.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef VERSION_H_
#define VERSION_H_

#include <stdint.h>

#define MAJOR_VERSION			0	/*　メジャーバージョン (0〜9)	*/
#define MINOR_VERSION			0	/*　マイナーバージョン (0〜99)	*/
#define REV_VERSION				0	/* Revision    (0～15)*/

#define BCD_VERSION				(((MAJOR_VERSION / 10) << 12) |		\
								 ((MAJOR_VERSION % 10) <<  8) |		\
								 ((MINOR_VERSION / 10) <<  4) |		\
								 ((MINOR_VERSION % 10) <<  0))

#endif /* VERSION_H_ */
