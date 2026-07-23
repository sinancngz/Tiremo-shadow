/**
 *******************************************************************************
 * @file        tiremo_mic_bsp.c
 * @brief       Tiremo microphone BSP implementation
 *
 * @details     ADC/Timer init: user_adc.c + user_timer1.c (MCUbrew).
 *              Capture sequence (Start):
 *              1. ApplyCaptureRuntime — MR.ADEN + MR.ARST (register, not SetConfig)
 *              2. SetCaptureIrqPoll → Timer1 Start → HAL_ADC_Start → ArmDma
 *
 *              Runtime: main loop calls Service() → ReadPendingEvents (DMAF) →
 *              IrqHandler → registered DMA handler → re-arm DMA.
 *
 * @note        Register access via tiremo_mic_adc_reg.h (CMSIS, not hal_adc_prv).
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_mic_bsp.h"

#include "hal_adc.h"
#include "hal_pcu.h"
#include "tiremo_mic_adc_reg.h"

#if (CONFIG_HAL_TIMER1 == 1)
#include "hal_timer1.h"
#endif

#if defined(_DMAC) && defined(DMA_ADC_NUM)
#include "hpl_dma.h"
#endif

extern void ADC_IRQHandler_ADC_ID_0(uint32_t un32Event, void *pContext);
extern uint32_t SystemCoreClock;

#if !defined(_DMAC)
#warning Tiremo mic DMA needs CONFIG_HAL_DMAC=1 (MCUbrew module or -DCONFIG_HAL_DMAC=1)
#endif

#define TIREMO_MIC_CAP_NONE     (0U)
#define TIREMO_MIC_CAP_DMA      (1U)
#define TIREMO_MIC_CAP_INTR     (2U)

/* --- Module state --------------------------------------------------------- */
static uint16_t s_dmaBuffer[TIREMO_MIC_DMA_BLOCK_SIZE] __attribute__((aligned(4)));
static volatile uint32_t s_adcIrqCount = 0U;
static volatile uint32_t s_dmaBlockCount = 0U;
static volatile uint32_t s_lastAdcStatus = 0U;
static volatile uint32_t s_lastStartStep = 0U;
static uint32_t s_captureMode = TIREMO_MIC_CAP_NONE;
static bool s_captureActive = false;
static TiremoMicBspDmaBlockFn_t s_pfnDmaBlock = NULL;
static TiremoMicBspSampleFn_t s_pfnSample = NULL;

void TIREMO_MIC_BSP_SetDataHandlers(const TiremoMicBspDataHandlers_t *pHandlers)
{
    if (pHandlers == NULL)
    {
        s_pfnDmaBlock = NULL;
        s_pfnSample = NULL;
        return;
    }

    s_pfnDmaBlock = pHandlers->pfnDmaBlock;
    s_pfnSample = pHandlers->pfnSample;
}

#if defined(_DMAC) && defined(DMA_ADC_NUM)
static DMA_ID_e s_dmaCh = DMA_ID_MAX;
static bool s_dmaBackendReady = false;
static bool s_dmaHwConfigured = false;
#endif

/* --- Debug trace (weak hook) ---------------------------------------------- */

__attribute__((weak)) void TIREMO_MIC_BSP_DbgStep(uint32_t step)
{
    (void)step;
}

static void TIREMO_MIC_BSP_Dbg(uint32_t step)
{
    s_lastStartStep = step;
    TIREMO_MIC_BSP_DbgStep(step);
}

#if (CONFIG_HAL_TIMER1 == 1)
static uint16_t TIREMO_MIC_BSP_CalcGra(uint32_t sampleRateHz)
{
    uint32_t timerClk;
    uint32_t period;

    timerClk = SystemCoreClock / 2U;
    period = timerClk / sampleRateHz;
    if (period > 0U)
    {
        period--;
    }
    if (period > 0xFFFFU)
    {
        period = 0xFFFFU;
    }

    return (uint16_t)period;
}
#endif

/**
 * @brief Switch ADC IRQ to poll mode for capture (MCUbrew init uses INTR_DMA).
 *        Not a duplicate of Init/Clk/Seq — required for DMAF polling in Service().
 */
static bool TIREMO_MIC_BSP_SetCaptureIrqPoll(void)
{
    HAL_ERR_e eErr;

    eErr = HAL_ADC_SetIRQ(TIREMO_MIC_ADC_ID,
                          ADC_OPS_POLL,
                          ADC_IRQHandler_ADC_ID_0,
                          NULL,
                          3U);
    return (eErr == HAL_ERR_OK);
}

/**
 * @brief Wake ADC and enable auto-restart for continuous capture.
 * @note  Does not call HAL_ADC_SetConfig / SetSeqConfig — those are in user_adc.c.
 */
static void TIREMO_MIC_BSP_ApplyCaptureRuntime(void)
{
    TIREMO_MIC_AdcReg_t *ptAdc = TIREMO_MIC_BSP_GetAdcReg();

    TIREMO_MIC_BSP_AdcSetEnable(ptAdc, true);
    TIREMO_MIC_BSP_AdcSetAutoRestart(ptAdc, true);
}

/* --- HPL DMA backend ------------------------------------------------------ */

static bool TIREMO_MIC_BSP_DmaBackendInit(void)
{
#if defined(_DMAC) && defined(DMA_ADC_NUM)
    HAL_ERR_e eErr;
    DMA_ID_e eId;

    if (s_dmaBackendReady)
    {
        return true;
    }

    eErr = HPL_DMA_Init(DMA_PERI_ADC, (uint8_t)TIREMO_MIC_ADC_ID);
    if (eErr != HAL_ERR_OK)
    {
        return false;
    }

    eErr = HPL_DMA_GetAvailChannel(&eId);
    if (eErr != HAL_ERR_OK)
    {
        return false;
    }

    s_dmaCh = eId;
    s_dmaBackendReady = true;
    return true;
#else
    return false;
#endif
}

static bool TIREMO_MIC_BSP_ArmDma(void)
{
#if defined(_DMAC) && defined(DMA_ADC_NUM)
    HAL_ERR_e eErr;
    TIREMO_MIC_AdcReg_t *ptAdc;
    DMA_CFG_t tDmaCfg =
    {
        .ePeri = DMA_PERI_ADC,
        .ePeriId = (DMA_PERI_ID_e)TIREMO_MIC_ADC_ID,
        .un32PeriNum = 0U,
        .eSize = DMA_BUS_SIZE_HALFWORD,
        .eDir = DMA_DIR_PERI_TO_MEM
    };

    if (!TIREMO_MIC_BSP_DmaBackendInit())
    {
        return false;
    }

    ptAdc = TIREMO_MIC_BSP_GetAdcReg();

    eErr = (HAL_ERR_e)HPL_DMA_GetPeriSelectNumber(DMA_PERI_ADC,
                                                  (DMA_PERI_ID_e)TIREMO_MIC_ADC_ID,
                                                  DMA_PERI_COM_RX,
                                                  &tDmaCfg.un32PeriNum);
    if (eErr != HAL_ERR_OK)
    {
        return false;
    }

    if (!s_dmaHwConfigured)
    {
        eErr = (HAL_ERR_e)HPL_DMA_SetConfig(s_dmaCh, &tDmaCfg);
    }
    else
    {
        (void)HPL_DMA_Stop(s_dmaCh);
        s_dmaHwConfigured = false;
        eErr = (HAL_ERR_e)HPL_DMA_SetConfig(s_dmaCh, &tDmaCfg);
    }
    if (eErr != HAL_ERR_OK)
    {
        return false;
    }
    s_dmaHwConfigured = true;

    TIREMO_MIC_BSP_AdcSetDmaEn(ptAdc, true);
    TIREMO_MIC_BSP_AdcSetDmaIe(ptAdc, true);

    if (TIREMO_MIC_BSP_IsDmaFlagSet(ptAdc))
    {
        TIREMO_MIC_BSP_ClearDmaFlag(ptAdc);
    }

    eErr = (HAL_ERR_e)HPL_DMA_Start(s_dmaCh,
                                    TIREMO_MIC_BSP_AdcGetDdrAddr(ptAdc),
                                    (uint32_t)s_dmaBuffer,
                                    TIREMO_MIC_DMA_BLOCK_SIZE);
    return (eErr == HAL_ERR_OK);
#else
    return false;
#endif
}

static void TIREMO_MIC_BSP_DmaTeardown(void)
{
#if defined(_DMAC) && defined(DMA_ADC_NUM)
    TIREMO_MIC_AdcReg_t *ptAdc;

    if (!s_dmaBackendReady)
    {
        return;
    }

    ptAdc = TIREMO_MIC_BSP_GetAdcReg();
    TIREMO_MIC_BSP_AdcSetDmaEn(ptAdc, false);
    TIREMO_MIC_BSP_AdcSetDmaIe(ptAdc, false);
    (void)HPL_DMA_Stop(s_dmaCh);
    s_dmaHwConfigured = false;
#endif
}

static void TIREMO_MIC_BSP_AdcIrqMask(bool bEnable)
{
    IRQn_Type eIrq;

    eIrq = TIREMO_MIC_BSP_GetAdcIrq();
    if (bEnable)
    {
        NVIC_ClearPendingIRQ(eIrq);
        NVIC_EnableIRQ(eIrq);
    }
    else
    {
        NVIC_DisableIRQ(eIrq);
        NVIC_ClearPendingIRQ(eIrq);
    }
}

/* --- ADC flag polling (DMAF not demuxed by HAL IRQ) ----------------------- */

static uint32_t TIREMO_MIC_BSP_ReadPendingEvents(void)
{
    TIREMO_MIC_AdcReg_t *ptAdc;
    uint32_t un32Event = 0U;

    ptAdc = TIREMO_MIC_BSP_GetAdcReg();
    s_lastAdcStatus = TIREMO_MIC_BSP_AdcGetSr(ptAdc);

    if (s_captureMode == TIREMO_MIC_CAP_DMA)
    {
        if (TIREMO_MIC_BSP_IsDmaFlagSet(ptAdc))
        {
            TIREMO_MIC_BSP_ClearDmaFlag(ptAdc);
            un32Event |= (uint32_t)ADC_EVENT_SINGLE_CAPTURED;
        }
        return un32Event;
    }

    if (TIREMO_MIC_BSP_IsDmaFlagSet(ptAdc))
    {
        TIREMO_MIC_BSP_ClearDmaFlag(ptAdc);
        un32Event |= (uint32_t)ADC_EVENT_SINGLE_CAPTURED;
    }

    if ((un32Event & (uint32_t)ADC_EVENT_SINGLE_CAPTURED) == 0U)
    {
        if (TIREMO_MIC_BSP_AdcIsSeqFlag(ptAdc))
        {
            TIREMO_MIC_BSP_AdcAckSeqFlag(ptAdc);
            un32Event |= (uint32_t)ADC_EVENT_BURST_CAPTURED;
        }
        else if (TIREMO_MIC_BSP_AdcIsSingleFlag(ptAdc))
        {
            TIREMO_MIC_BSP_AdcAckSingleFlag(ptAdc, false);
            un32Event |= (uint32_t)ADC_EVENT_BURST_CAPTURED;
        }
    }

    return un32Event;
}

/* --- Public API ----------------------------------------------------------- */

bool TIREMO_MIC_BSP_SetSampleRate(uint32_t sampleRateHz)
{
#if (CONFIG_HAL_TIMER1 == 1)
    HAL_ERR_e eErr;
    uint16_t gra;

    if (s_captureActive)
    {
        return false;
    }

    if ((sampleRateHz == 0U) || (sampleRateHz > TIREMO_MIC_FREQ_48KHZ))
    {
        return false;
    }

#if (TIREMO_MIC_TIMER_MCUBREW == 1)
    gra = TIREMO_MIC_BSP_CalcGra(sampleRateHz);
    eErr = HAL_TIMER1_SetData(TIREMO_MIC_ADC_TIMER_ID, TIMER1_DATA_A, (uint32_t)gra);
    return (eErr == HAL_ERR_OK);
#else
    (void)sampleRateHz;
    return false;
#endif
#else
    (void)sampleRateHz;
    return false;
#endif
}

void TIREMO_MIC_Init(void)
{
    HAL_ERR_e eErr;

    eErr = HAL_PCU_SetInOutMode(TIREMO_MIC_PORT, TIREMO_MIC_PIN, PCU_INOUT_ANG_INPUT);
    if (eErr != HAL_ERR_OK)
    {
        return;
    }

    (void)HAL_PCU_SetAltMode(TIREMO_MIC_PORT, TIREMO_MIC_PIN, PCU_ALT_7);
}

bool TIREMO_MIC_BSP_Start(void)
{
    HAL_ERR_e eErr;
    uint32_t primask;

    TIREMO_MIC_BSP_Dbg(1U);

    if (s_captureActive)
    {
        return true;
    }

    primask = (uint32_t)__get_PRIMASK();
    __disable_irq();
    TIREMO_MIC_BSP_AdcIrqMask(false);
    TIREMO_MIC_BSP_Dbg(2U);

    if (!TIREMO_MIC_BSP_SetCaptureIrqPoll())
    {
        TIREMO_MIC_BSP_Dbg(90U);
        if (primask == 0U)
        {
            __enable_irq();
        }
        return false;
    }
    TIREMO_MIC_BSP_Dbg(3U);

    TIREMO_MIC_BSP_ApplyCaptureRuntime();
    TIREMO_MIC_BSP_Dbg(4U);

    s_captureMode = TIREMO_MIC_CAP_INTR;

    s_captureActive = true;
    TIREMO_MIC_BSP_Dbg(7U);

#if (CONFIG_HAL_TIMER1 == 1)
    (void)HAL_TIMER1_Start(TIREMO_MIC_ADC_TIMER_ID);
#endif
    TIREMO_MIC_BSP_Dbg(8U);

    eErr = HAL_ADC_Start(TIREMO_MIC_ADC_ID);
    if (eErr != HAL_ERR_OK)
    {
#if (CONFIG_HAL_TIMER1 == 1)
        (void)HAL_TIMER1_Stop(TIREMO_MIC_ADC_TIMER_ID);
#endif
        s_captureActive = false;
        s_captureMode = TIREMO_MIC_CAP_NONE;
        TIREMO_MIC_BSP_Dbg(91U);
        if (primask == 0U)
        {
            __enable_irq();
        }
        return false;
    }
    TIREMO_MIC_BSP_Dbg(9U);

#if (TIREMO_MIC_USE_DMA == 1)
    if (TIREMO_MIC_BSP_ArmDma())
    {
        TIREMO_MIC_BSP_Dbg(5U);
        s_captureMode = TIREMO_MIC_CAP_DMA;
    }
#endif

    TIREMO_MIC_BSP_Dbg(10U);
    if (primask == 0U)
    {
        __enable_irq();
    }
    return true;
}

void TIREMO_MIC_BSP_Stop(void)
{
    if (!s_captureActive)
    {
        return;
    }

    TIREMO_MIC_BSP_AdcIrqMask(false);

    if (s_captureMode == TIREMO_MIC_CAP_DMA)
    {
        TIREMO_MIC_BSP_DmaTeardown();
    }

#if (CONFIG_HAL_TIMER1 == 1)
    (void)HAL_TIMER1_Stop(TIREMO_MIC_ADC_TIMER_ID);
#endif

    (void)HAL_ADC_Stop(TIREMO_MIC_ADC_ID);
    s_captureActive = false;
    s_captureMode = TIREMO_MIC_CAP_NONE;
}

void TIREMO_MIC_BSP_Service(void)
{
    uint32_t un32Event;

    if (!s_captureActive)
    {
        return;
    }

    un32Event = TIREMO_MIC_BSP_ReadPendingEvents();
    if (un32Event != 0U)
    {
        TIREMO_MIC_BSP_IrqHandler(un32Event, NULL);
    }
}

void TIREMO_MIC_BSP_IrqHandler(uint32_t un32Event, void *pContext)
{
    (void)pContext;

    if (un32Event == 0U)
    {
        return;
    }

    s_adcIrqCount++;

    if ((un32Event & (uint32_t)ADC_EVENT_SINGLE_CAPTURED) != 0U)
    {
        s_dmaBlockCount++;
        if (s_pfnDmaBlock != NULL)
        {
            s_pfnDmaBlock(s_dmaBuffer, TIREMO_MIC_DMA_BLOCK_SIZE);
        }

        if (s_captureActive && (s_captureMode == TIREMO_MIC_CAP_DMA))
        {
            (void)TIREMO_MIC_BSP_ArmDma();
        }
        return;
    }

    if ((un32Event & (uint32_t)ADC_EVENT_BURST_CAPTURED) != 0U)
    {
        ADC_SEQ_DATA_t tData;

        tData.bReadDDR = (s_captureMode == TIREMO_MIC_CAP_DMA);
        if (HAL_ADC_GetData(TIREMO_MIC_ADC_ID, 0U, &tData) == HAL_ERR_OK)
        {
            if (s_pfnSample != NULL)
            {
                s_pfnSample(tData.un16Result);
            }
        }
    }
}

uint32_t TIREMO_MIC_BSP_GetIrqCount(void)
{
    return s_adcIrqCount;
}

uint32_t TIREMO_MIC_BSP_GetDmaBlockCount(void)
{
    return s_dmaBlockCount;
}

uint32_t TIREMO_MIC_BSP_GetAdcStatus(void)
{
    return s_lastAdcStatus;
}

uint32_t TIREMO_MIC_BSP_GetCaptureMode(void)
{
    return s_captureMode;
}

uint32_t TIREMO_MIC_BSP_GetLastStartStep(void)
{
    return s_lastStartStep;
}

const uint16_t *TIREMO_MIC_BSP_GetDmaBuffer(void)
{
    return s_dmaBuffer;
}

uint32_t TIREMO_MIC_BSP_GetDmaBlockSize(void)
{
    return TIREMO_MIC_DMA_BLOCK_SIZE;
}
