/**
 *******************************************************************************
 * @file        tiremo_mic_board.h
 * @brief       Board configuration — Tiremo microphone BSP on AUDK32
 *
 * @details     Hardware: PA0 (AN0) → ADC CH0, Timer1 ID.0 trigger @ 16 kHz.
 *              Capture path: burst ADC + HPL DMA block transfer + SR.DMAF poll.
 *
 *              BSP hardware constants only. APP settings: tiremo_mic_app.h.
 *
 * @note        Do not edit in Tiremo (MCUbrew regenerates): user_adc.c, main.c,
 *              user_timer1.c, DebugLibrary/, ProductConfig/.
 *              BSP sets MR.ADEN + MR.ARST at capture Start (ApplyCaptureRuntime).
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_MIC_BOARD_H_
#define TIREMO_MIC_BOARD_H_

#include "hal_adc.h"
#include "hal_pcu.h"

#if (CONFIG_HAL_TIMER1 == 1)
#include "hal_timer1.h"
#endif

/* ========================================================================== */
/* Hardware mapping (must match MCUbrew adc_configuration.yaml)              */
/* ========================================================================== */

#define TIREMO_MIC_ADC_ID               ADC_ID_0
#define TIREMO_MIC_ADC_CHANNEL          (0U)
#define TIREMO_MIC_ADC_SEQ_NUM          (0U)
#define TIREMO_MIC_ADC_TIMER_TRG_NUM    (0U)
#define TIREMO_MIC_ADC_SAMPLING_TIME    (8U)
#define TIREMO_MIC_ADC_TIMER_ID         TIMER1_ID_0

#define TIREMO_MIC_PORT                 PCU_ID_A
#define TIREMO_MIC_PIN                  PCU_PIN_ID_0

/** 1 = HPL DMA + ADC SR.DMAF poll; 0 = per-sample EOS interrupt path. */
#define TIREMO_MIC_USE_DMA              (1)

/** Samples per DMA block (must match HPL_DMA_Start count in BSP). */
#define TIREMO_MIC_DMA_BLOCK_SIZE       (64U)

/** DMA DDR halfword → 12-bit sample (hardware left-aligns in DDR). */
#define TIREMO_MIC_ADC_RAW_FROM_DDR(h) \
    ((uint16_t)(((uint16_t)(h) & 0xFFF0U) >> 4U))

/* ========================================================================== */
/* Sample rates                                                               */
/* ========================================================================== */

#define TIREMO_MIC_FREQ_8KHZ            (8000U)
#define TIREMO_MIC_FREQ_16KHZ           (16000U)
#define TIREMO_MIC_FREQ_32KHZ           (32000U)
#define TIREMO_MIC_FREQ_48KHZ           (48000U)

/** Timer1 clock/init from user_timer1.c; BSP only sets GRA and Start/Stop. */
#define TIREMO_MIC_TIMER_MCUBREW        (1)

#endif /* TIREMO_MIC_BOARD_H_ */
