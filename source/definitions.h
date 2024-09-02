/*
 * definitions.h
 *
 *  Created on: 2020/06/30
 *      Author: akino
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

/*--- 続いている内の何れかを定義する ---*/
// 下記で定義することにして、ビルド構成で選択することにしました。
// Properties -> C/C++ Build -> Settings -> Tool Settings -> Preprocessor -> Defined symbols (-D)

//#define BOARD_EVK			// 元祖EVK
//#define BOARD_EVK_SLAVE	// EVKでSAIをSLAVEにする
//#define BOARD_PROTO1		// PROTO1

/*--- 必要に応じて定義する ---*/

#define LOCAL_DEBUG_ENABLE	// 定義するとMIDIを傍受してデバッグモニタに入る
//#define GPIO_11_OUT_ENABLE	// 定義するとGPIO_11をOUT設定してデバッグで使う
#define ADC_ENABLE			// 定義するとADC有効
//#define UART_LANE_ENABLE_R2	// 定義すると対ULZでLANE release 2 が有効
//#define TIMING_MEASURRING	// 定義するとtiming logが有効

/*--- 以下は、上記の定義を受けて定義される。 ---*/

#if defined(BOARD_EVK)||defined(BOARD_EVK_SLAVE)
#define I2C_GPIO_01_02
#define AUDIO_PLL_ENABLE
#endif	//defined(BOARD_EVK)||defined(BOARD_EVK_SLAVE)

#ifdef BOARD_EVK_SLAVE
#define WM8960_MASTER	// WM8960をMASTER設定
#define SAI_SLAVE		// SAIをSLAVE設定
#define BOARD_UART_BAUDRATE (4000000U)
//#define BOARD_UART_BAUDRATE (31250U)	//*
#define VERSION_SET
//#define HIGHSPEEDONLY
#define SAI_TDM
#endif	//SAI_EVK_SLAVE

#ifdef BOARD_PROTO1
//#define I2C_GPIO_11_12
#ifdef GPIO_11_OUT_ENABLE
#undef GPIO_11_OUT_ENABLE
#define GPIO_13_OUT_ENABLE
#endif	//GPIO_11_OUT_ENABLE
#define SAI_SLAVE		// SAIをSLAVE設定
#define SAI_RX_ASYNC
#ifdef BOARD_USE_CODEC
#undef BOARD_USE_CODEC
#endif	//BOARD_USE_CODEC
#define USE_LPUART1
#define USE_LPUART3
#define USE_LPUART4
#define BOARD_UART_BAUDRATE (4000000U)
#define SERIAL_FLASH_IS25LP032D
#define BOARD_FLASH_SIZE (0x400000U)
#define VERSION_SET
#define PADSW45CHANGE
#define USE_EDMA
//#define HIGHSPEEDONLY
#define SAI_TDM
#else	//BOARD_PROTO1
#define USE_LPUART1
#define SERIAL_FLASH_AT25SF128A
#endif	//BOARD_PROTO1

#endif /* DEFINITIONS_H_ */
