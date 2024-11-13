/*
 * p_adc.c
 *
 *  Created on: 2024/11/05
 *      Author: koki_arai
 */

/*
 * p_adc.h
 *
 *  Created on: 2024/11/01
 *      Author: koki_arai
 */


#if (defined __CORTEX_M) && ((__CORTEX_M == 4U) || (__CORTEX_M == 7U))
#define SDK_ISR_EXIT_BARRIER __DSB()
#else
#define SDK_ISR_EXIT_BARRIER
#endif

#include "p_adc.h"
#include "fsl_adc.h"
#include "fsl_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ADC_BASE			ADC1
#define ADC_IRQn			ADC1_IRQn
#define ADC_CHANNEL_GROUP	0U
#define ADC_IRQHandler		ADC1_IRQHandler

#define ARRAYSIZE(aaa) (sizeof(aaa)/sizeof(aaa[0]))

typedef struct {
	uint8_t	channel;	// adc channel (GPIO_AD_"○○")
} ADSEL_t;

static ADSEL_t adsel[] = {
	{1},
	{2}
};

static volatile bool g_AdcConversionDoneFlag;
static volatile uint8_t g_AdcInterruptCounter = 0;
static volatile uint16_t g_AdcConversionValue[ARRAYSIZE(adsel)] = {0};
static volatile uint8_t g_AdcConversionChannelFlag[ARRAYSIZE(adsel)] = {0};

/*
* ADC interrupt request : Called when a single channel( = single gpio) has been comleteled.
* Andstart conversion of next chaneel. 
*/
void ADC_IRQHandler(void)
{
    /* Read conversion result to clear the conversion completed flag. */
		g_AdcConversionDoneFlag = true;
    g_AdcConversionValue[g_AdcInterruptCounter] = ADC_GetChannelConversionValue(ADC_BASE, ADC_CHANNEL_GROUP);
    g_AdcConversionChannelFlag[g_AdcInterruptCounter] = 1;
		g_AdcInterruptCounter++;
    if (g_AdcInterruptCounter == ARRAYSIZE(adsel)){
    	g_AdcInterruptCounter = 0;
			
    }else{
			adcInitiateConversion(g_AdcInterruptCounter);
		}
    SDK_ISR_EXIT_BARRIER;
    return;
}

/**
 * Set up ADC.
 * Contain execution of auto caliburation 
 */
void adcInit(void)
{
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

	if (kStatus_Success == ADC_DoAutoCalibration(ADC_BASE)){
		//dprintf(SDIR_UARTMIDI, "\nADC_DoAutoCalibration() Done.");
	}else{
		//dprintf(SDIR_UARTMIDI, "\nADC_DoAutoCalibration() Failed.");
	}
	g_AdcConversionDoneFlag = true;	// for 1st start

	return;
}

/*
* Order adc to run conversion.
*/
void adcInitiateConversion(int select)
{
	g_AdcConversionDoneFlag = false;
	adc_channel_config_t adcChannelConfigStruct;
	adcChannelConfigStruct.channelNumber                        = adsel[select].channel;
	adcChannelConfigStruct.enableInterruptOnConversionCompleted = true;
	ADC_SetChannelConfig(ADC_BASE, ADC_CHANNEL_GROUP, &adcChannelConfigStruct);
	return;
}

/*
* Initiate conversion. 
* In other words, adc interrupt will be running untill all channel has been completed.
*/
void adcStartFillingOnelap(){
	if(!g_AdcConversionDoneFlag)return;
	g_AdcConversionDoneFlag = false;
	adcInitiateConversion(0);
}


int16_t adcGetValue(int index)
{
	int16_t ret = -1;

	if (index < ARRAYSIZE(g_AdcConversionValue))
	{
		ret = g_AdcConversionValue[index];
	}

	return ret;
}

uint32_t adcGetFlag(int index1st, int index2nd)
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

void adcClearFlag(int index1st, int index2nd)
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

