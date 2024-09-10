/*
 * adc.c
 *
 *  Created on: 2020/09/09
 *      Author: akino
 */

#include "fsl_adc.h"
#include "fsl_gpio.h"
#include "adc.h"

//#include "../application/MidiDebugMonitor/MidiDebugMonitor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//#define USE_ADC_ETC	// 定義するとADC_ETCを使用する。

#define ADC_BASE			ADC1
#define ADC_IRQn			ADC1_IRQn
#define ADC_CHANNEL_GROUP	0U
#define ADC_IRQHandler		ADC1_IRQHandler

#ifdef USE_ADC_ETC
#define ADC_ETC_CHAIN_LENGTH  0U
#define ADC_ETC_DONE0_Handler ADC_ETC_IRQ0_IRQHandler
#define ADC_ETC_DONE1_Handler ADC_ETC_IRQ1_IRQHandler
#define ADC_ETC_DONE2_Handler ADC_ETC_IRQ2_IRQHandler
#define ADC_ETC_DONE3_Handler ADC_ETC_IRQ3_IRQHandler
#endif	//USE_ADC_ETC

#if (defined __CORTEX_M) && ((__CORTEX_M == 4U) || (__CORTEX_M == 7U))
#define SDK_ISR_EXIT_BARRIER __DSB()
#else
#define SDK_ISR_EXIT_BARRIER
#endif

#ifdef BOARD_PROTO1
#define ADSEL0PIN	(0)
#else	//BOARD_PROTO1
#define ADSEL0PIN	(15)
#define ADSEL1PIN	(0)
#endif	//BOARD_PROTO1

typedef struct {
	uint8_t adchn;	// gpio pin
	uint8_t	sel0;	// gpio group
#ifndef BOARD_PROTO1
	uint8_t	sel1;
#endif	//BOARD_PROTO1
} ADSEL_t;

#define ARRAYSIZE(aaa) (sizeof(aaa)/sizeof(aaa[0]))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
//void jobTimeStart(int index);
//void jobTimeStop(int index);
//void jobTimeInterval(int index);


/*******************************************************************************
 * Variables
 ******************************************************************************/
static ADSEL_t adsel[] = {
#ifdef BOARD_PROTO1
	{ 0, 0},	//  0
	{ 0, 0},	//  1 ExtPad46
	{ 1, 0},	//  2 ExtPad48
	{ 5, 0},	//  3 CCPadLeft42
	{ 6, 0},	//  4 CCPadCRight44
	{ 7, 0},	//  5 Pad38
	{ 8, 0},	//	6 HihatPedal

	{ 9, 0},	//  7 Pad40
	{10, 0},	//  8 Pad36
	{11, 0},	//  9
	{11, 0},	// 10 CCRibbonLeft
	{14, 0},	// 11
	{14, 0},	// 12 CCRibbonCRight
	{ 2, 0},	// 13 SwitchPedal1

	{ 0, 1},	// 14
	{ 0, 1},	// 15 ExtPad47
	{ 1, 1},	// 16 ExtPad49
	{ 5, 1},	// 17 CCPadCLeft43
	{ 6, 1},	// 18 CCPadRight45
	{ 7, 1},	// 19 Pad39
	{ 3, 1},	// 20 SwitchPedal2

	{ 9, 1},	// 21 Pad41
	{10, 1},	// 22 Pad37
	{11, 1},	// 23
	{11, 1},	// 24 CCRibbonCLeft
	{14, 1},	// 25
	{14, 1},	// 26 CCRibbonRight
	{ 4, 1},	// 27 VolumePedal

	{ 0, 2},	// 28 PadSw38
	{ 1, 2},	// 29 PadSw39
	{ 2, 2},	// 30 PadSw40
#ifdef PADSW45CHANGE
	{10, 1},	// 31 PadSw36
	{ 9, 1},	// 32 PadSw41
#else	//PADSW45CHANGE
	{ 3, 2},	// 31 PadSw36
	{ 4, 2},	// 32 PadSw41
#endif	//PADSW45CHANGE
	{ 5, 2},	// 33 PadSw37
#else	//BOARD_PROTO1
	{ 0, 0},	//  0
	{ 15,1}
#endif	//BOARD_PROTO1
};

#ifdef USE_ADC_ETC
#ifdef BOARD_PROTO1
#define ADC_ETC_GRP0_OFS	(0)
#define ADC_ETC_GRP0_CNT	(7)
#define ADC_ETC_GRP1_OFS	(7)
#define ADC_ETC_GRP1_CNT	(7)
#define ADC_ETC_GRP2_OFS	(14)
#define ADC_ETC_GRP2_CNT	(7)
#define ADC_ETC_GRP3_OFS	(21)
#define ADC_ETC_GRP3_CNT	(7)
#define GPIO_OFS			(28)
#define GPIO_CNT			(6)
#else	//BOARD_PROTO1
#define ADC_ETC_GRP0_OFS	(0)
#define ADC_ETC_GRP0_CNT	(3)
#define ADC_ETC_GRP1_OFS	(3)
#define ADC_ETC_GRP1_CNT	(3)
#define ADC_ETC_GRP2_OFS	(6)
#define ADC_ETC_GRP2_CNT	(3)
#define ADC_ETC_GRP3_OFS	(9)
#define ADC_ETC_GRP3_CNT	(3)
#endif	//BOARD_PROTO1
#endif	//USE_ADC_ETC

static volatile bool g_AdcConversionDoneFlag;
static volatile uint8_t g_AdcInterruptCounter = 0;
static volatile uint16_t g_AdcConversionValue[ARRAYSIZE(adsel)] = {0};
static volatile uint8_t g_AdcConversionChannelFlag[ARRAYSIZE(adsel)] = {0};

bool g_bAdcCalib = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

#ifdef USE_ADC_ETC
void ADC_ETC_DONE0_Handler(void)
{
	ADC_ETC_ClearInterruptStatusFlags(ADC_ETC, kADC_ETC_Trg0TriggerSource, kADC_ETC_Done0StatusFlagMask);
	++g_AdcInterruptCounter;
	g_AdcConversionDoneFlag = true;
	for (int i = 0; i < ADC_ETC_GRP0_CNT; i++)
	{
		g_AdcConversionValue[ADC_ETC_GRP0_OFS+i] = ADC_ETC_GetADCConversionValue(ADC_ETC, 0U, i); /* Get trigger0 chainX result. */
		g_AdcConversionChannelFlag[ADC_ETC_GRP0_OFS+i] = 1;
	}
#ifdef GPIO_11_OUT_ENABLE
//	GPIO_PortToggle(GPIO1, 1<<11);
#endif	//GPIO_11_OUT_ENABLE
	adc_start(g_AdcInterruptCounter);
	SDK_ISR_EXIT_BARRIER;
}
void ADC_ETC_DONE1_Handler(void)
{
	ADC_ETC_ClearInterruptStatusFlags(ADC_ETC, kADC_ETC_Trg1TriggerSource, kADC_ETC_Done1StatusFlagMask);
	++g_AdcInterruptCounter;
	g_AdcConversionDoneFlag = true;
	for (int i = 0; i < ADC_ETC_GRP1_CNT; i++)
	{
		g_AdcConversionValue[ADC_ETC_GRP1_OFS+i] = ADC_ETC_GetADCConversionValue(ADC_ETC, 1U, i); /* Get trigger0 chainX result. */
		g_AdcConversionChannelFlag[ADC_ETC_GRP1_OFS+i] = 1;
	}
#ifdef GPIO_11_OUT_ENABLE
//	GPIO_PortToggle(GPIO1, 1<<11);
#endif	//GPIO_11_OUT_ENABLE
	adc_start(g_AdcInterruptCounter);
	SDK_ISR_EXIT_BARRIER;
}
void ADC_ETC_DONE2_Handler(void)
{
	ADC_ETC_ClearInterruptStatusFlags(ADC_ETC, kADC_ETC_Trg2TriggerSource, kADC_ETC_Done2StatusFlagMask);
	++g_AdcInterruptCounter;
	g_AdcConversionDoneFlag = true;
	for (int i = 0; i < ADC_ETC_GRP2_CNT; i++)
	{
		g_AdcConversionValue[ADC_ETC_GRP2_OFS+i] = ADC_ETC_GetADCConversionValue(ADC_ETC, 2U, i); /* Get trigger0 chainX result. */
		g_AdcConversionChannelFlag[ADC_ETC_GRP2_OFS+i] = 1;
	}
#ifdef GPIO_11_OUT_ENABLE
//	GPIO_PortToggle(GPIO1, 1<<11);
#endif	//GPIO_11_OUT_ENABLE
	adc_start(g_AdcInterruptCounter);
	SDK_ISR_EXIT_BARRIER;
}
void ADC_ETC_DONE3_Handler(void)
{
	ADC_ETC_ClearInterruptStatusFlags(ADC_ETC, kADC_ETC_Trg3TriggerSource, kADC_ETC_Done3StatusFlagMask);
	g_AdcInterruptCounter = 0;
	g_AdcConversionDoneFlag = true;
	for (int i = 0; i < ADC_ETC_GRP3_CNT; i++)
	{
		g_AdcConversionValue[ADC_ETC_GRP3_OFS+i] = ADC_ETC_GetADCConversionValue(ADC_ETC, 3U, i); /* Get trigger0 chainX result. */
		g_AdcConversionChannelFlag[ADC_ETC_GRP3_OFS+i] = 1;
	}
#ifdef GPIO_11_OUT_ENABLE
	GPIO_PortToggle(GPIO1, 1<<11);
#endif	//GPIO_11_OUT_ENABLE
#ifdef GPIO_13_OUT_ENABLE
	GPIO_PortToggle(GPIO1, 1<<13);
#endif	//GPIO_13_OUT_ENABLE
	//jobTimeStop(1);
#ifdef GPIO_11_OUT_ENABLE
   	GPIO_PortClear(GPIO1, 1<<11);
#endif	//GPIO_11_OUT_ENABLE
#ifdef GPIO_13_OUT_ENABLE
   	GPIO_PortClear(GPIO1, 1<<13);
#endif	//GPIO_13_OUT_ENABLE
	SDK_ISR_EXIT_BARRIER;
}
#else	//USE_ADC_ETC
void ADC_IRQHandler(void)
{
    g_AdcConversionDoneFlag = true;
    /* Read conversion result to clear the conversion completed flag. */
    g_AdcConversionValue[g_AdcInterruptCounter] = ADC_GetChannelConversionValue(ADC_BASE, ADC_CHANNEL_GROUP);
    g_AdcConversionChannelFlag[g_AdcInterruptCounter] = 1;
#ifdef GPIO_11_OUT_ENABLE
    GPIO_PortToggle(GPIO1, 1<<11);
#endif	//GPIO_11_OUT_ENABLE
    if (++g_AdcInterruptCounter == ARRAYSIZE(adsel))
    {
    	g_AdcInterruptCounter = 0;
    	//jobTimeStop(1);
#ifdef GPIO_11_OUT_ENABLE
    	GPIO_PortClear(GPIO1, 1<<11);
#endif	//GPIO_11_OUT_ENABLE
    }
    else
    {
    	adc_start(g_AdcInterruptCounter);
    }
    SDK_ISR_EXIT_BARRIER;

    return;
}
#endif	//USE_ADC_ETC

#ifdef USE_ADC_ETC
static void adc_etc_trigger_init(int grp, ADSEL_t *pAdsel, int cnt)
{
	adc_etc_trigger_config_t adcEtcTriggerConfig;
	adc_etc_trigger_chain_config_t adcEtcTriggerChainConfig;

	/* Set the external XBAR trigger0 configuration. */
	adcEtcTriggerConfig.enableSyncMode      = false;
	adcEtcTriggerConfig.enableSWTriggerMode = true;
	adcEtcTriggerConfig.triggerChainLength  = cnt-1;//ADC_ETC_CHAIN_LENGTH; /* Chain length is count -1. */
	adcEtcTriggerConfig.triggerPriority     = 0U;
	adcEtcTriggerConfig.sampleIntervalDelay = 0U;
	adcEtcTriggerConfig.initialDelay        = 0U;
	ADC_ETC_SetTriggerConfig(ADC_ETC, grp, &adcEtcTriggerConfig);

	for (int i = 0; i < cnt; i++)
	{
		/* Set the external XBAR trigger(ch) chain(i) configuration. */
		adcEtcTriggerChainConfig.enableB2BMode       = i == 0 ? false : true;
		adcEtcTriggerChainConfig.ADCHCRegisterSelect = 1U << grp; /* Select ADC_HCn register to trigger. */
		adcEtcTriggerChainConfig.ADCChannelSelect = pAdsel[i].adchn;//ADC_ETC_CHANNEL; /* ADC_HC0 will be triggered to sample Corresponding channel. */
		adcEtcTriggerChainConfig.InterruptEnable = grp;//kADC_ETC_Done0InterruptEnable; /* Enable the Done0 interrupt. */
#if defined(FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN) && FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN
		adcEtcTriggerChainConfig.enableIrq = i != cnt-1 ? false : true; /* Enable the IRQ. */
#endif                                         /* FSL_FEATURE_ADC_ETC_HAS_TRIGm_CHAIN_a_b_IEn_EN */
		ADC_ETC_SetTriggerChainConfig(ADC_ETC, grp, i, &adcEtcTriggerChainConfig); /* Configure the trigger0 chain0. */
	}

	return;
}
#endif	//USE_ADC_ETC

void adc_init(void)
{
#ifdef USE_ADC_ETC
	{	// ADC part
	    adc_config_t adcConfig;
	    adc_channel_config_t adcChannelConfigStruct;

	    /* Enable ADC_ACLK_EN clock gate. */
	    CCM->CSCMR2 |= CCM_CSCMR2_ADC_ACLK_EN_MASK;

	    /* Initialize the ADC module. */
	    ADC_GetDefaultConfig(&adcConfig);
	    adcConfig.enableHighSpeed = true;
	    adcConfig.clockSource = kADC_ClockSourceALT;
	    ADC_Init(ADC_BASE, &adcConfig);
	    ADC_EnableHardwareTrigger(ADC_BASE, true);

	    adcChannelConfigStruct.channelNumber = 16; /* 16 is External channel selection from ADC_ETC. */
	    adcChannelConfigStruct.enableInterruptOnConversionCompleted = false;
	    ADC_SetChannelConfig(ADC_BASE, 0, &adcChannelConfigStruct);
	    ADC_SetChannelConfig(ADC_BASE, 1, &adcChannelConfigStruct);
	    ADC_SetChannelConfig(ADC_BASE, 2, &adcChannelConfigStruct);
	    ADC_SetChannelConfig(ADC_BASE, 3, &adcChannelConfigStruct);

	    /* Do auto hardware calibration. */
	    if (kStatus_Success == ADC_DoAutoCalibration(ADC_BASE))
#if 1
	    {
	    	g_bAdcCalib = true;
	    }
#else
	    {
	        dprintf(SDIR_UARTMIDI, "ADC_DoAutoCalibration() Done.\r\n");
	    }
	    else
	    {
	        dprintf(SDIR_UARTMIDI, "ADC_DoAutoCalibration() Failed.\r\n");
	    }
#endif
	}

	{	// ADC_ETC part
		adc_etc_config_t adcEtcConfig;

		/* Initialize the ADC_ETC. */
		ADC_ETC_GetDefaultConfig(&adcEtcConfig);
		adcEtcConfig.XBARtriggerMask = 7U; /* Enable the external XBAR trigger0~2. */
		ADC_ETC_Init(ADC_ETC, &adcEtcConfig);

		adc_etc_trigger_init(0, &adsel[ADC_ETC_GRP0_OFS], ADC_ETC_GRP0_CNT);
		adc_etc_trigger_init(1, &adsel[ADC_ETC_GRP1_OFS], ADC_ETC_GRP1_CNT);
		adc_etc_trigger_init(2, &adsel[ADC_ETC_GRP2_OFS], ADC_ETC_GRP2_CNT);
		adc_etc_trigger_init(3, &adsel[ADC_ETC_GRP3_OFS], ADC_ETC_GRP3_CNT);

#if 0
		/*  */
	    NVIC_SetPriority(ADC_ETC_IRQ0_IRQn, 0);
	    NVIC_SetPriority(ADC_ETC_IRQ1_IRQn, 0);
	    NVIC_SetPriority(ADC_ETC_IRQ2_IRQn, 0);
	    NVIC_SetPriority(ADC_ETC_IRQ3_IRQn, 0);
#endif

		/* Enable the NVIC. */
		EnableIRQ(ADC_ETC_IRQ0_IRQn);
		EnableIRQ(ADC_ETC_IRQ1_IRQn);
		EnableIRQ(ADC_ETC_IRQ2_IRQn);
		EnableIRQ(ADC_ETC_IRQ3_IRQn);
	}
#else	//USE_ADC_ETC
	adc_config_t adcConfigStruct;
	NVIC_SetPriority(ADC_IRQn, 5U);
	EnableIRQ(ADC_IRQn);

	/* Enable ADC_ACLK_EN clock gate. */
	CCM->CSCMR2 |= CCM_CSCMR2_ADC_ACLK_EN_MASK;

	ADC_GetDefaultConfig(&adcConfigStruct);
	adcConfigStruct.enableHighSpeed = true;
    adcConfigStruct.clockSource = kADC_ClockSourceALT;
	ADC_Init(ADC_BASE, &adcConfigStruct);
	ADC_EnableHardwareTrigger(ADC_BASE, false);
	
	if (kStatus_Success == ADC_DoAutoCalibration(ADC_BASE))
	{
		//dprintf(SDIR_UARTMIDI, "\nADC_DoAutoCalibration() Done.");
	}
	else
	{
		//dprintf(SDIR_UARTMIDI, "\nADC_DoAutoCalibration() Failed.");
	}
#endif	//USE_ADC_ETC

    g_AdcConversionDoneFlag = true;	// for 1st start

	return;
}

void adc_start(int select)
{
	if (select == 0)
	{	// from timer
		//jobTimeInterval(0);

		//jobTimeStart(1);
#ifdef BOARD_PROTO1
		for (int i = 0; i < GPIO_CNT; i++)
		{
			int bit = adsel[GPIO_OFS+i].adchn;
			GPIO_Type *base = adsel[GPIO_OFS+i].sel0 == 1 ? GPIO1 : GPIO2;
			int value = GPIO_PinRead(base, bit) ? 4095 : 0;

			g_AdcConversionValue[GPIO_OFS+i] = value;
			g_AdcConversionChannelFlag[GPIO_OFS+i] = 1;
		}
#endif	//BOARD_PROTO1
	}
	if (select || ((select == 0) && g_AdcConversionDoneFlag))
	{
		g_AdcConversionDoneFlag = false;
#ifdef USE_ADC_ETC

		switch (select)
		{
		case 0:
			GPIO_PinWrite(GPIO1, ADSEL0PIN, adsel[ADC_ETC_GRP0_OFS].sel0);
#ifndef BOARD_PROTO1
			GPIO_PinWrite(GPIO1, ADSEL1PIN, adsel[ADC_ETC_GRP0_OFS].sel1);
#endif	//BOARD_PROTO1
			break;
		case 1:
			GPIO_PinWrite(GPIO1, ADSEL0PIN, adsel[ADC_ETC_GRP1_OFS].sel0);
#ifndef BOARD_PROTO1
			GPIO_PinWrite(GPIO1, ADSEL1PIN, adsel[ADC_ETC_GRP1_OFS].sel1);
#endif	//BOARD_PROTO1
			break;
		case 2:
			GPIO_PinWrite(GPIO1, ADSEL0PIN, adsel[ADC_ETC_GRP2_OFS].sel0);
#ifndef BOARD_PROTO1
			GPIO_PinWrite(GPIO1, ADSEL1PIN, adsel[ADC_ETC_GRP2_OFS].sel1);
#endif	//BOARD_PROTO1
			break;
		case 3:
			GPIO_PinWrite(GPIO1, ADSEL0PIN, adsel[ADC_ETC_GRP3_OFS].sel0);
#ifndef BOARD_PROTO1
			GPIO_PinWrite(GPIO1, ADSEL1PIN, adsel[ADC_ETC_GRP3_OFS].sel1);
#endif	//BOARD_PROTO1
			break;
		default:
			break;
		}

		ADC_ETC_DoSoftwareTrigger(ADC_ETC, select); /* Do software XBAR trigger0. */
#else	//USE_ADC_ETC
		adc_channel_config_t adcChannelConfigStruct;

		//GPIO_PinWrite(GPIO1, ADSEL0PIN, adsel[select].sel0);
#ifndef BOARD_PROTO1
		//GPIO_PinWrite(GPIO1, ADSEL1PIN, adsel[select].sel1);
#endif	//BOARD_PROTO1

		/* Configure the user channel and interrupt. */
//		adcChannelConfigStruct.channelNumber                        = ADC_USER_CHANNEL;
		adcChannelConfigStruct.channelNumber                        = adsel[select].sel0;
		adcChannelConfigStruct.enableInterruptOnConversionCompleted = true;

		ADC_SetChannelConfig(ADC_BASE, ADC_CHANNEL_GROUP, &adcChannelConfigStruct);
#endif	//USE_ADC_ETC
	}

	return;
}

int16_t adc_getValue(int index)
{
	int16_t ret = -1;

	if (index < ARRAYSIZE(g_AdcConversionValue))
	{
		ret = g_AdcConversionValue[index];
	}

	return ret;
}

uint32_t adc_getFlag(int index1st, int index2nd)
{
	uint32_t ret =  0;

	if ((index1st >= 0) && (index2nd >= 0))
	{
		ret = g_AdcConversionChannelFlag[index1st] & g_AdcConversionChannelFlag[index2nd];
	}
	else if (index1st >= 0)
	{
		ret = g_AdcConversionChannelFlag[index1st];
	}
	else if (index2nd >= 0)
	{
		ret = g_AdcConversionChannelFlag[index2nd];
	}

	return ret;
}

void adc_clrFlag(int index1st, int index2nd)
{
	if (index1st >= 0)
	{
		g_AdcConversionChannelFlag[index1st] = 0;
	}
	if (index2nd >= 0)
	{
		g_AdcConversionChannelFlag[index2nd] = 0;
	}

	return;
}
