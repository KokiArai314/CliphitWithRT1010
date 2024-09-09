/*
 * sai.c
 *
 *  Created on: 2024/09/09
 *      Author: koki_arai
 */


#include "sai.h"

#include "fsl_wm8960.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_dmamux.h"
#include "fsl_iomuxc.h"

/* SAI instance and clock */
#define CODEC_VOLUME     (100)	//0x18U
#define CODEC_SAI              SAI1
#define CODEC_SAI_CHANNEL      (0)
#define CODEC_SAI_IRQ          SAI1_IRQn
#define CODEC_SAITxIRQHandler  SAI1_IRQHandler
#define CODEC_SAI_TX_SYNC_MODE kSAI_ModeAsync
#define CODEC_SAI_RX_SYNC_MODE kSAI_ModeSync
#define CODEC_SAI_MCLK_OUTPUT  true
#define CODEC_SAI_MASTER_SLAVE kSAI_Master
#define CODEC_SAI_UserIRQHandler    SAI1_IRQHandler

/* IRQ */
#define CODEC_SAI_TX_IRQ SAI1_IRQn
#define CODEC_SAI_RX_IRQ SAI1_IRQn

/* DMA */
#define CODEC_DMA             DMA0
#define CODEC_DMAMUX          DMAMUX
#define CODEC_TX_EDMA_CHANNEL (0U)
#define CODEC_RX_EDMA_CHANNEL (1U)
#define CODEC_SAI_TX_SOURCE   kDmaRequestMuxSai1Tx
#define CODEC_SAI_RX_SOURCE   kDmaRequestMuxSai1Rx

/* Select Audio/Video PLL (786.432 MHz) as sai1 clock source */
#define CODEC_SAI1_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define CODEC_SAI1_CLOCK_SOURCE_PRE_DIVIDER (3U)
/* Clock divider for sai1 clock source */
#define CODEC_SAI1_CLOCK_SOURCE_DIVIDER (15U)
/* Get frequency of sai1 clock: SAI1_Clock = 786.432MHz /(3+1)/(15+1) = 12.288MHz */
#define CODEC_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (CODEC_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (CODEC_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void txCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);
static void rxCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);


/*******************************************************************************
 * Variables
 ******************************************************************************/
wm8960_config_t wm8960Config = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
	.leftInputSource = kWM8960_InputSingleEndedMic,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    //48000 * 24bit * 8 = 8467200
    .format = {.mclk_HZ = 8467200U, .sampleRate = CODEC_AUDIO_SAMPLE_RATE, .bitWidth = CODEC_AUDIO_BIT_WIDTH},
    .master_slave = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};

/// @note support 44100Hz
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 47,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 4,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator = 1,    /* 30 bit numerator of fractional loop divider. */
    .denominator = 25, /* 30 bit denominator of fractional loop divider */
};

AT_NONCACHEABLE_SECTION_INIT(sai_edma_handle_t txHandle) = {0};
edma_handle_t dmaTxHandle                                = {0};
AT_NONCACHEABLE_SECTION_INIT(sai_edma_handle_t rxHandle) = {0};
edma_handle_t dmaRxHandle                                = {0};
AT_NONCACHEABLE_SECTION_INIT(sai_edma_handle_t mqsHandle) = {0};

AT_NONCACHEABLE_SECTION_ALIGN(uint8_t audioBuff[BUFFER_SIZE * BUFFER_NUM], 4);

extern codec_config_t boardCodecConfig;

volatile uint32_t fullBlock    = 0;
volatile uint32_t emptyBlock   = BUFFER_NUM-1;	// -1 for previous transfer
volatile uint32_t mqsBlock     = 0;

sai_transceiver_t saiConfig;
codec_handle_t codecHandle;

static TaskHandle_t sai_task_handle;

float pfWork[AudioChannel][NumOfSlot];
float *ppfWork[] = {&(pfWork[0][0]), &(pfWork[1][0])};

int saiTxError = 0;
int saiRxError = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void sai_init(void){
    /* Init DMAMUX */
    DMAMUX_Init(CODEC_DMAMUX);
    DMAMUX_SetSource(CODEC_DMAMUX, CODEC_TX_EDMA_CHANNEL, (uint8_t)CODEC_SAI_TX_SOURCE);
    DMAMUX_EnableChannel(CODEC_DMAMUX, CODEC_TX_EDMA_CHANNEL);
    DMAMUX_SetSource(CODEC_DMAMUX, CODEC_RX_EDMA_CHANNEL, (uint8_t)CODEC_SAI_RX_SOURCE);
    DMAMUX_EnableChannel(CODEC_DMAMUX, CODEC_RX_EDMA_CHANNEL);

    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(CODEC_DMA, &dmaConfig);
    EDMA_CreateHandle(&dmaTxHandle, CODEC_DMA, CODEC_TX_EDMA_CHANNEL);
    EDMA_CreateHandle(&dmaRxHandle, CODEC_DMA, CODEC_RX_EDMA_CHANNEL);
    EDMA_CreateHandle(&dmaMqsHandle, MQS_DMA, MQS_EDMA_CHANNEL);

    /* SAI init */
    SAI_Init(CODEC_SAI);
    SAI_Init(MQS_SAI);

    SAI_TransferTxCreateHandleEDMA(CODEC_SAI, &txHandle, txCallback, NULL, &dmaTxHandle);
    SAI_TransferRxCreateHandleEDMA(CODEC_SAI, &rxHandle, rxCallback, NULL, &dmaRxHandle);
    SAI_TransferTxCreateHandleEDMA(MQS_SAI, &mqsHandle, mqsCallback, NULL, &dmaMqsHandle);

    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&saiConfig, CODEC_AUDIO_BIT_WIDTH, kSAI_Stereo, 1U << CODEC_SAI_CHANNEL);
    saiConfig.syncMode    = CODEC_SAI_TX_SYNC_MODE;
    saiConfig.masterSlave = CODEC_SAI_MASTER_SLAVE;
    SAI_TransferTxSetConfigEDMA(CODEC_SAI, &txHandle, &saiConfig);
    saiConfig.syncMode = CODEC_SAI_RX_SYNC_MODE;
    SAI_TransferRxSetConfigEDMA(CODEC_SAI, &rxHandle, &saiConfig);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(CODEC_SAI, CODEC_AUDIO_MASTER_CLOCK, CODEC_AUDIO_SAMPLE_RATE, CODEC_AUDIO_BIT_WIDTH,
                          CODEC_AUDIO_DATA_CHANNEL);
    SAI_RxSetBitClockRate(CODEC_SAI, CODEC_AUDIO_MASTER_CLOCK, CODEC_AUDIO_SAMPLE_RATE, CODEC_AUDIO_BIT_WIDTH,
                          CODEC_AUDIO_DATA_CHANNEL);

    
    /* Use default setting to init codec */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }
    CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                    CODEC_VOLUME);

    SAI_TxEnableInterrupts(CODEC_SAI, kSAI_FIFOErrorInterruptEnable);
    SAI_RxEnableInterrupts(CODEC_SAI, kSAI_FIFOErrorInterruptEnable);
    EnableIRQ(CODEC_SAI_TX_IRQ);
    EnableIRQ(CODEC_SAI_RX_IRQ);

    return;

}

void sai_task(){
    sai_transfer_t xfer    = {0};
    SAI_TxSoftwareReset(CODEC_SAI, kSAI_ResetTypeSoftware);
    SAI_RxSoftwareReset(CODEC_SAI, kSAI_ResetTypeSoftware);

}
