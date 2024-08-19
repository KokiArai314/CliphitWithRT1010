/*
 * base_types.h
 *
 *  Created on: 2020/02/06
 *      Author: higuchi
 */

#ifndef BASE_TYPES_H_
#define BASE_TYPES_H_

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/

//#if __GNUC__	/* __GNUC__ */
//#include <stdio.h>
//#include <stdint.h>
//#else
//#include <stddef.h>
//#include <stdbool.h>
//#include "stdint.h"
//#endif
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "board.h"

///*****************************************************************************/
///* Global pre-processor symbols/macros ('#define')                           */
///*****************************************************************************/
//
//#ifndef TRUE
//  /** Value is true (boolean_t type) */
//  #define TRUE        ((boolean_t) 1)
//#endif
//
//#ifndef FALSE
//  /** Value is false (boolean_t type) */
//  #define FALSE       ((boolean_t) 0)
//#endif
//
///** Returns the minimum value out of two values */
//#define MIN( X, Y )  ((X) < (Y) ? (X) : (Y))
//
///** Returns the maximum value out of two values */
//#define MAX( X, Y )  ((X) > (Y) ? (X) : (Y))
//
///** Returns the dimension of an array */
//#define DIM( X )  (sizeof(X) / sizeof(X[0]))

/*****************************************************************************/

#define REG_R(reg)                (reg)             /* read from register */
#define REG_W(reg, value)         ((reg) = (value)) /* write to register  */

#define B(b)    (((0x##b##ULL & 0x000000000000000FULL) ? 0x0001U : 0) | \
                 ((0x##b##ULL & 0x00000000000000F0ULL) ? 0x0002U : 0) | \
                 ((0x##b##ULL & 0x0000000000000F00ULL) ? 0x0004U : 0) | \
                 ((0x##b##ULL & 0x000000000000F000ULL) ? 0x0008U : 0) | \
                 ((0x##b##ULL & 0x00000000000F0000ULL) ? 0x0010U : 0) | \
                 ((0x##b##ULL & 0x0000000000F00000ULL) ? 0x0020U : 0) | \
                 ((0x##b##ULL & 0x000000000F000000ULL) ? 0x0040U : 0) | \
                 ((0x##b##ULL & 0x00000000F0000000ULL) ? 0x0080U : 0) | \
                 ((0x##b##ULL & 0x0000000F00000000ULL) ? 0x0100U : 0) | \
                 ((0x##b##ULL & 0x000000F000000000ULL) ? 0x0200U : 0) | \
                 ((0x##b##ULL & 0x00000F0000000000ULL) ? 0x0400U : 0) | \
                 ((0x##b##ULL & 0x0000F00000000000ULL) ? 0x0800U : 0) | \
                 ((0x##b##ULL & 0x000F000000000000ULL) ? 0x1000U : 0) | \
                 ((0x##b##ULL & 0x00F0000000000000ULL) ? 0x2000U : 0) | \
                 ((0x##b##ULL & 0x0F00000000000000ULL) ? 0x4000U : 0) | \
                 ((0x##b##ULL & 0xF000000000000000ULL) ? 0x8000U : 0))

#define MAKE16(h, l)    (((uint16_t)((h) & 0xff) << 8) | (uint16_t)((l) & 0xff))
#define MAKE32(h, l)    (((uint32_t)((h) & 0xffff) << 16) | (uint32_t)((l) & 0xffff))
#define L32(ll)         ((uint32_t)(((uint64_t)(ll)) & 0xffffffff))
#define H32(ll)         ((uint32_t)((((uint64_t)(ll)) >> 32) & 0xffffffff))
#define L16(l)          ((uint16_t)(((uint32_t)(l)) & 0xffff))
#define H16(l)          ((uint16_t)((((uint32_t)(l)) >> 16) & 0xffff))
#define L8(w)           ((uint8_t)(((uint16_t)(w)) & 0xff))
#define H8(w)           ((uint8_t)((((uint16_t)(w)) >> 8) & 0xff))

#ifdef	__BIG_ENDIAN
#define BI8(w)           H8(w), L8(w)
#define QD8(l)           BI8(H16(l)), BI8(L16(l))
#define BI16(l)          H16(l), L16(l)
#else
#define BI8(w)           L8(w), H8(w)
#define QD8(l)           BI8(L16(l)), BI8(H16(l))
#define BI16(l)          L16(l), H16(l)
#endif

#ifdef	__BIG_ENDIAN
#define RD16(adr)        ((uint16_t)((*((uint8_t*)adr) << 8) + (*((uint8_t*)adr+1) & 0xFF)))
#define WR16(adr,w)      *adr = H8(w); *(adr+1) = L8(w)
#else
#define RD16(adr)        ((uint16_t)((*((uint8_t*)adr) & 0xFF) + (*((uint8_t*)adr+1) << 8)))
#define WR16(adr,w)      *adr = L8(w); *(adr+1) = H8(w)
#endif

#define	xdefbit(bufname, bitno) bufname.f.b##bitno
#define	defbit(bufname, bitno) xdefbit(bufname, bitno)
#define absdelta(op1, op2) ((op1) > (op2) ? (op1) - (op2) : (op2) - (op1))

///******************************************************************************
// * Global type definitions
// ******************************************************************************/
//
///** logical datatype (only values are TRUE and FALSE) */
//typedef uint8_t      boolean_t;
//
///** single precision floating point number (4 byte) */
//typedef float        float32_t;
//
///** double precision floating point number (8 byte) */
//typedef double       float64_t;
//
///** ASCCI character for string generation (8 bit) */
//typedef char         char_t;
//
///** function pointer type to void/void function */
//typedef void         (*func_ptr_t)(void);
//
///** function pointer type to void/uint8_t function */
//typedef void         (*func_ptr_arg1_t)(uint8_t);
//
///** generic error codes */
//typedef enum en_result
//{
//    Ok                          = 0,  ///< No error
//    Error                       = 1,  ///< Non-specific error code
//    ErrorAddressAlignment       = 2,  ///< Address alignment does not match
//    ErrorAccessRights           = 3,  ///< Wrong mode (e.g. user/system) mode is set
//    ErrorInvalidParameter       = 4,  ///< Provided parameter is not valid
//    ErrorOperationInProgress    = 5,  ///< A conflicting or requested operation is still in progress
//    ErrorInvalidMode            = 6,  ///< Operation not allowed in current mode
//    ErrorUninitialized          = 7,  ///< Module (or part of it) was not initialized properly
//    ErrorBufferFull             = 8,  ///< Circular buffer can not be written because the buffer is full
//    ErrorTimeout                = 9,  ///< Time Out error occurred (e.g. I2C arbitration lost, Flash time-out, etc.)
//    ErrorNotReady               = 10, ///< A requested final state is not reached
//    OperationInProgress         = 11  ///< Indicator for operation in progress (e.g. ADC conversion not finished, DMA channel used, etc.)
//} en_result_t;

/*****************************************************************************/

typedef struct {
	uint16_t	min;
	uint16_t	max;
} range16_t;

typedef union {
	uint16_t	u16;
	struct {
#ifdef	__BIG_ENDIAN
		uint8_t	h;
		uint8_t	l;
#else
		uint8_t	l;
		uint8_t	h;
#endif
	} u8;
} u16u8_t;

typedef union {
	int16_t	i16;
	struct {
#ifdef	__BIG_ENDIAN
		uint8_t	h;
		uint8_t	l;
#else
		uint8_t	l;
		uint8_t	h;
#endif
	} u8;
} i16u8_t;

typedef union {
	uint32_t	u32;
	struct {
#ifdef	__BIG_ENDIAN
		uint8_t	hh;
		uint8_t	hl;
		uint8_t	lh;
		uint8_t	ll;
#else
		uint8_t	ll;
		uint8_t	lh;
		uint8_t	hl;
		uint8_t	hh;
#endif
	} u8;
} u32u8_t;

typedef union {
	uint32_t	u32;
	struct {
#ifdef	__BIG_ENDIAN
		uint16_t	h;
		uint16_t	l;
#else
		uint16_t	l;
		uint16_t	h;
#endif
	} u16;
} u32u16_t;

typedef union {
	int32_t	i32;
	struct {
#ifdef	__BIG_ENDIAN
		uint8_t	hh;
		uint8_t	hl;
		uint8_t	lh;
		uint8_t	ll;
#else
		uint8_t	ll;
		uint8_t	lh;
		uint8_t	hl;
		uint8_t	hh;
#endif
	} u8;
} i32u8_t;

typedef union {
	uint8_t	byte;
	struct {
		uint8_t	b0:1;
		uint8_t	b1:1;
		uint8_t	b2:1;
		uint8_t	b3:1;
		uint8_t	b4:1;
		uint8_t	b5:1;
		uint8_t	b6:1;
		uint8_t	b7:1;
	} f;
} fbuf_t;

typedef union {
	uint16_t	word;
	struct {
		uint16_t	b0:1;
		uint16_t	b1:1;
		uint16_t	b2:1;
		uint16_t	b3:1;
		uint16_t	b4:1;
		uint16_t	b5:1;
		uint16_t	b6:1;
		uint16_t	b7:1;
		uint16_t	b8:1;
		uint16_t	b9:1;
		uint16_t	b10:1;
		uint16_t	b11:1;
		uint16_t	b12:1;
		uint16_t	b13:1;
		uint16_t	b14:1;
		uint16_t	b15:1;
	} f;
} fbufw_t;

typedef union {
	uint32_t	word;
	struct {
		uint32_t	b0:1;
		uint32_t	b1:1;
		uint32_t	b2:1;
		uint32_t	b3:1;
		uint32_t	b4:1;
		uint32_t	b5:1;
		uint32_t	b6:1;
		uint32_t	b7:1;
		uint32_t	b8:1;
		uint32_t	b9:1;
		uint32_t	b10:1;
		uint32_t	b11:1;
		uint32_t	b12:1;
		uint32_t	b13:1;
		uint32_t	b14:1;
		uint32_t	b15:1;
		uint32_t	b16:1;
		uint32_t	b17:1;
		uint32_t	b18:1;
		uint32_t	b19:1;
		uint32_t	b20:1;
		uint32_t	b21:1;
		uint32_t	b22:1;
		uint32_t	b23:1;
		uint32_t	b24:1;
		uint32_t	b25:1;
		uint32_t	b26:1;
		uint32_t	b27:1;
		uint32_t	b28:1;
		uint32_t	b29:1;
		uint32_t	b30:1;
		uint32_t	b31:1;
	} f;
} fbuf32_t;

#endif /* BASE_TYPES_H_ */
