/**
 *******************************************************************************
 * @file        tiremo_mic_adc_reg.h
 * @brief       CMSIS ADC register helpers for BSP (no hal_adc_prv.h)
 *
 * @details     HAL private headers are not on the TmplUserApp include path after
 *              MCUbrew regenerate. These inline helpers mirror the macros in
 *              hal_adc_prv.h using CMSIS ADC0_Type from abov_config.h.
 *
 *              Used by tiremo_mic_bsp.c for:
 *              - DMA enable / interrupt enable (MR, IER)
 *              - DDR source address for HPL_DMA_Start
 *              - SR.DMAF / EOSIF / EOCIF poll and acknowledge
 *              - MR.ADEN / MR.ARST for capture runtime (ApplyCaptureRuntime)
 *
 * @note        Instance selected by TIREMO_MIC_ADC_ID in tiremo_mic_board.h.
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_MIC_ADC_REG_H_
#define TIREMO_MIC_ADC_REG_H_

#include "abov_config.h"
#include "tiremo_mic_board.h"

/* --- ADC instance mapping ------------------------------------------------- */

#if (TIREMO_MIC_ADC_ID == ADC_ID_0)
#define TIREMO_MIC_ADC_REG_DEV          ADC0
#define TIREMO_MIC_ADC_IRQn             ADC0_IRQn
typedef ADC0_Type TIREMO_MIC_AdcReg_t;
#elif (TIREMO_MIC_ADC_ID == ADC_ID_1)
#define TIREMO_MIC_ADC_REG_DEV          ADC1
#define TIREMO_MIC_ADC_IRQn             ADC1_IRQn
typedef ADC1_Type TIREMO_MIC_AdcReg_t;
#elif (TIREMO_MIC_ADC_ID == ADC_ID_2)
#define TIREMO_MIC_ADC_REG_DEV          ADC2
#define TIREMO_MIC_ADC_IRQn             ADC2_IRQn
typedef ADC2_Type TIREMO_MIC_AdcReg_t;
#else
#error "tiremo_mic_adc_reg.h: unsupported TIREMO_MIC_ADC_ID"
#endif

/* --- Base pointer and NVIC ------------------------------------------------ */

static inline TIREMO_MIC_AdcReg_t *TIREMO_MIC_BSP_GetAdcReg(void)
{
    return TIREMO_MIC_ADC_REG_DEV;
}

static inline IRQn_Type TIREMO_MIC_BSP_GetAdcIrq(void)
{
    return TIREMO_MIC_ADC_IRQn;
}

/* --- Capture runtime (MR — no HAL_ADC_SetConfig) -------------------------- */

static inline void TIREMO_MIC_BSP_AdcSetEnable(TIREMO_MIC_AdcReg_t *ptAdc, bool bEnable)
{
    if (bEnable)
    {
        ptAdc->MR |= ADC_MR_ADEN_Msk;
    }
    else
    {
        ptAdc->MR &= ~ADC_MR_ADEN_Msk;
    }
}

static inline void TIREMO_MIC_BSP_AdcSetAutoRestart(TIREMO_MIC_AdcReg_t *ptAdc, bool bEnable)
{
    if (bEnable)
    {
        ptAdc->MR |= ADC_MR_ARST_Msk;
    }
    else
    {
        ptAdc->MR &= ~ADC_MR_ARST_Msk;
    }
}

/* --- DMA control (MR.DMAEN, IER.DMAIE) ------------------------------------ */

static inline void TIREMO_MIC_BSP_AdcSetDmaEn(TIREMO_MIC_AdcReg_t *ptAdc, bool bEnable)
{
    if (bEnable)
    {
        ptAdc->MR |= ADC_MR_DMAEN_Msk;
    }
    else
    {
        ptAdc->MR &= ~ADC_MR_DMAEN_Msk;
    }
}

static inline void TIREMO_MIC_BSP_AdcSetDmaIe(TIREMO_MIC_AdcReg_t *ptAdc, bool bEnable)
{
    if (bEnable)
    {
        ptAdc->IER |= ADC_IER_DMAIE_Msk;
    }
    else
    {
        ptAdc->IER &= ~ADC_IER_DMAIE_Msk;
    }
}

static inline uint32_t TIREMO_MIC_BSP_AdcGetDdrAddr(const TIREMO_MIC_AdcReg_t *ptAdc)
{
    return (uint32_t)&ptAdc->DDR;
}

/* --- Status flags (SR) ---------------------------------------------------- */

static inline bool TIREMO_MIC_BSP_IsDmaFlagSet(const TIREMO_MIC_AdcReg_t *ptAdc)
{
    return ((ptAdc->SR & ADC_SR_DMAF_Msk) != 0U);
}

static inline void TIREMO_MIC_BSP_ClearDmaFlag(TIREMO_MIC_AdcReg_t *ptAdc)
{
    ptAdc->SR = ADC_SR_DMAF_Msk;
}

static inline uint32_t TIREMO_MIC_BSP_AdcGetSr(const TIREMO_MIC_AdcReg_t *ptAdc)
{
    return ptAdc->SR;
}

static inline bool TIREMO_MIC_BSP_AdcIsSeqFlag(const TIREMO_MIC_AdcReg_t *ptAdc)
{
    return ((ptAdc->SR & ADC_SR_EOSIF_Msk) != 0U);
}

static inline void TIREMO_MIC_BSP_AdcAckSeqFlag(TIREMO_MIC_AdcReg_t *ptAdc)
{
    ptAdc->SR = ADC_SR_EOSIF_Msk;
}

static inline bool TIREMO_MIC_BSP_AdcIsSingleFlag(const TIREMO_MIC_AdcReg_t *ptAdc)
{
    return ((ptAdc->SR & ADC_SR_EOCIF_Msk) != 0U);
}

static inline void TIREMO_MIC_BSP_AdcAckSingleFlag(TIREMO_MIC_AdcReg_t *ptAdc, bool bData)
{
    ptAdc->SR = ((uint32_t)bData << ADC_SR_EOCIF_Pos);
}

#endif /* TIREMO_MIC_ADC_REG_H_ */
