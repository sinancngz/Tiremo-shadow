/**
 *******************************************************************************
 * @file        tiremo_mic_bsp.h
 * @brief       Tiremo microphone BSP — hardware capture layer
 *
 * @details     Responsibilities:
 *              - GPIO: PA0 analog input (TIREMO_MIC_Init)
 *              - ADC/Timer init from user_adc.c / user_timer1.c (MCUbrew)
 *              - MR.ADEN + MR.ARST at Start (ApplyCaptureRuntime)
 *              - HPL DMA arm / SR.DMAF poll / re-arm (main-loop Service)
 *              - Timer1 GRA for sample rate (clock from user_timer1.c)
 *              - Sample delivery via registered callbacks (SetDataHandlers)
 *
 *              Capture timing: Timer1 hardware trigger starts each ADC burst
 *              (no Timer1 IRQ required — bIntrEnable = False in user_timer1.c).
 *              Data ready: DMA complete flag (SR.DMAF) polled from the main loop
 *              via TIREMO_MIC_BSP_Service() — not ADC_IRQHandler.
 *
 * @note        user_adc_isr.c stays empty; capture events are polled via Service().
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_MIC_BSP_H_
#define TIREMO_MIC_BSP_H_

#include <stdbool.h>
#include <stdint.h>

#include "tiremo_mic_board.h"

/** @brief DMA block ready — raw halfwords from BSP DMA buffer. */
typedef void (*TiremoMicBspDmaBlockFn_t)(const uint16_t *pRawSamples,
                                         uint32_t sampleCount);

/** @brief Single sample (interrupt fallback path). */
typedef void (*TiremoMicBspSampleFn_t)(uint16_t rawSample);

/** @brief Handlers registered by the application layer (APP Init). */
typedef struct
{
    TiremoMicBspDmaBlockFn_t pfnDmaBlock;
    TiremoMicBspSampleFn_t pfnSample;
} TiremoMicBspDataHandlers_t;

/**
 * @brief   Register sample callbacks. NULL clears handlers.
 * @note    Call from APP Init before Start. BSP never calls APP symbols directly.
 */
void TIREMO_MIC_BSP_SetDataHandlers(const TiremoMicBspDataHandlers_t *pHandlers);

/**
 * @brief   Configure PA0 as analog input (call from APP Init).
 */
void TIREMO_MIC_Init(void);

/**
 * @brief       Set Timer1 period for the requested sample rate.
 * @param[in]   sampleRateHz  Hz (8k–48k); must not be called while capturing.
 * @return      true on success.
 */
bool TIREMO_MIC_BSP_SetSampleRate(uint32_t sampleRateHz);

/**
 * @brief   Start burst capture: ADC prep, Timer1, DMA arm.
 * @return  true on success.
 */
bool TIREMO_MIC_BSP_Start(void);

/** @brief Stop Timer1, ADC, and DMA. */
void TIREMO_MIC_BSP_Stop(void);

/**
 * @brief       ADC event handler (ISR or polled).
 * @param[in]   un32Event  ADC_EVENT_* bitmask from HAL or BSP poll.
 * @param[in]   pContext   Unused (HAL callback signature).
 */
void TIREMO_MIC_BSP_IrqHandler(uint32_t un32Event, void *pContext);

/**
 * @brief   Poll ADC SR.DMAF / EOS flags and dispatch IrqHandler.
 * @note    Call frequently from the main loop while capturing.
 */
void TIREMO_MIC_BSP_Service(void);

/** @brief Pointer to the internal DMA scratch buffer (debug / inspection). */
const uint16_t *TIREMO_MIC_BSP_GetDmaBuffer(void);

/** @brief Number of halfwords per DMA block (TIREMO_MIC_DMA_BLOCK_SIZE). */
uint32_t TIREMO_MIC_BSP_GetDmaBlockSize(void);

/** @brief Total IRQ / poll events handled since Start. */
uint32_t TIREMO_MIC_BSP_GetIrqCount(void);

/** @brief DMA blocks completed since Start. */
uint32_t TIREMO_MIC_BSP_GetDmaBlockCount(void);

/** @brief Last ADC SR snapshot (debug). */
uint32_t TIREMO_MIC_BSP_GetAdcStatus(void);

/** @brief Capture backend: 0=idle, 1=DMA, 2=INTR per-sample fallback. */
uint32_t TIREMO_MIC_BSP_GetCaptureMode(void);

/** @brief Last TIREMO_MIC_BSP_Dbg() step (boot failure diagnosis). */
uint32_t TIREMO_MIC_BSP_GetLastStartStep(void);

/**
 * @brief       Optional start trace hook (weak, no-op by default).
 * @details     Override in prv_user_code.c if you want UART boot-step trace.
 */
void TIREMO_MIC_BSP_DbgStep(uint32_t step);

#endif /* TIREMO_MIC_BSP_H_ */
