/**
 *******************************************************************************
 * @file        tiremo_microphone.h
 * @brief       Tiremo microphone library — public entry point
 *
 * @details     Layered library for the ADC_Microphone example on ABOV AUDK32:
 *
 *              @code
 *              Tiremo/microphone/
 *              ├── tiremo_microphone.h   (this file — include once)
 *              ├── bsp/
 *              │   ├── tiremo_mic_board.h    Pin / ADC / Timer / DMA config
 *              │   ├── tiremo_mic_adc_reg.h  CMSIS ADC register helpers
 *              │   ├── tiremo_mic_bsp.h/.c   GPIO, burst capture, DMA, Timer1
 *              └── app/
 *                  └── tiremo_mic_app.h/.c   Capture, voice alert, PCM stream reader
 *              @endcode
 *
 *              Typical integration:
 *              - Init / Start / Stop via APP API
 *              - TIREMO_MIC_BSP_Service() in the main loop (DMAF poll)
 *              - Voice logic in tiremo_mic_app.c; optional OnVoiceAlert in prv
 *
 *              MCUbrew-owned files (do not duplicate Init): user_adc.c,
 *              user_timer1.c. Capture uses ADC DMA flag polling in the main
 *              loop (TIREMO_MIC_BSP_Service) — not ADC_IRQHandler.
 *
 * @code
 * #include "Tiremo/microphone/tiremo_microphone.h"
 *
 * TIREMO_MIC_App_InitDefault();
 * TIREMO_MIC_App_Start();
 *
 * while (1) { TIREMO_MIC_BSP_Service(); }
 * @endcode
 *
 * @note        App defaults: tiremo_mic_app.h. BSP hardware: tiremo_mic_board.h.
 *              Custom buffer: GetDefaultConfig() + Init().
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_MICROPHONE_H_
#define TIREMO_MICROPHONE_H_

#include "bsp/tiremo_mic_bsp.h"
#include "app/tiremo_mic_app.h"

#endif /* TIREMO_MICROPHONE_H_ */
