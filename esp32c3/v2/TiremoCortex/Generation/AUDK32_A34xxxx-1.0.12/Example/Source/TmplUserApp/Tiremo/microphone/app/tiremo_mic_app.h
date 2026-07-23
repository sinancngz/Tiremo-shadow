/**
 *******************************************************************************
 * @file        tiremo_mic_app.h
 * @brief       Tiremo microphone application layer
 *
 * @details     User application layer — edit this file for your product logic:
 *              - Capture buffer, HP filter, RMS, HIGH / NORMAL / SILENCE detection
 *              - Buffer full → ProcessVoice() runs here automatically
 *
 *              Data flow:
 *              BSP (handler) → StoreSample() → HandleBufferReady()
 *                    → ProcessVoice() → OnVoiceAlert() (weak, optional UART in prv)
 *
 *              APP registers BSP callbacks in Init via TIREMO_MIC_BSP_SetDataHandlers().
 *
 * @note        BSP has no APP dependency. app.c includes bsp.h only.
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#ifndef TIREMO_MIC_APP_H_
#define TIREMO_MIC_APP_H_

#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */
/* Application configuration — edit for your product                         */
/* ========================================================================== */

#define TIREMO_MIC_DEFAULT_SAMPLE_RATE  (16000U)
#define TIREMO_MIC_APP_MAX_SAMPLE_RATE  (48000U)

/** Input volume 0–100 (TIREMO_MIC_DEFAULT_VOLUME). */
#define TIREMO_MIC_DEFAULT_VOLUME       (80U)

/** DC offset subtracted before the high-pass filter. */
#define TIREMO_MIC_ADC_OFFSET           (1705U)

/** 12-bit ADC result mask. */
#define TIREMO_MIC_ADC_MASK             (0x0FFFU)

/** Unsigned PCM centre: stored value = filtered_sample + TIREMO_MIC_PCM_CENTER. */
#define TIREMO_MIC_PCM_CENTER           (32768U)

#define TIREMO_MIC_ADC_CLAMP12(v)       ((uint16_t)((v) & TIREMO_MIC_ADC_MASK))

/** 0 = HP filter + gain → PCM@32768; 1 = store raw 12-bit ADC in buffer. */
#define TIREMO_MIC_APP_STORE_RAW_ADC    (0)

/** Digital gain applied after volume scaling in the HP filter path. */
#define TIREMO_MIC_SIGNAL_GAIN          (8U)

/** Circular capture buffer length (samples). */
#define TIREMO_MIC_APP_BUFFER_SIZE      (1024U)

/** Envelope attack divisor (smaller = faster rise). */
#define TIREMO_MIC_VOICE_LEVEL_ATTACK_DIV  (2U)

/** Envelope release divisor (larger = slower fall). */
#define TIREMO_MIC_VOICE_LEVEL_RELEASE_DIV (16U)

/** Consecutive loud buffers before HIGH VOICE. */
#define TIREMO_MIC_VOICE_LOUD_BUFFERS      (2U)

/** Consecutive quiet buffers before leaving HIGH or entering SILENCE. */
#define TIREMO_MIC_VOICE_NORMAL_BUFFERS    (8U)

/** Hysteresis: enter / exit smoothed RMS thresholds. */
#define TIREMO_MIC_VOICE_LOUD_ENTER        (1000U)
#define TIREMO_MIC_VOICE_LOUD_EXIT         (700U)
#define TIREMO_MIC_VOICE_SILENCE_ENTER     (180U)
#define TIREMO_MIC_VOICE_SILENCE_EXIT      (450U)

/** 1 = periodic RMS hook; 0 = voice alerts only. */
#define TIREMO_MIC_APP_DEBUG_RMS_UART      (0)

/** Every N full buffers when debug is enabled. */
#define TIREMO_MIC_APP_DEBUG_RMS_EVERY_N   (16U)

/** @brief Application state machine. */
typedef enum
{
    TIREMO_MIC_STATE_RESET = 0U,
    TIREMO_MIC_STATE_READY,
    TIREMO_MIC_STATE_RECORDING,
    TIREMO_MIC_STATE_ERROR
} TiremoMicState_e;

/** @brief IIR high-pass filter state (one channel). */
typedef struct
{
    int32_t z;
    int32_t oldOut;
    int32_t oldIn;
} TiremoMicHpFilter_t;

/** @brief Parameters passed to TIREMO_MIC_App_Init(). */
typedef struct
{
    uint32_t sampleRateHz;
    uint32_t volume;       /**< 0–100, scales input before gain */
    uint16_t *pBuffer;     /**< Caller-owned circular buffer */
    uint32_t bufferSize;
} TiremoMicAppConfig_t;

/** @brief Voice alert returned when state changes (NONE = no change this call). */
typedef enum
{
    TIREMO_MIC_ALERT_NONE = 0,
    TIREMO_MIC_ALERT_NORMAL,
    TIREMO_MIC_ALERT_HIGH_VOICE,
    TIREMO_MIC_ALERT_SILENCE
} TiremoMicAlert_e;

/** @brief Result of one buffer voice analysis. */
typedef struct
{
    TiremoMicAlert_e alert;
    uint32_t level;
} TiremoMicVoiceResult_t;

/**
 * @brief Fill @p pConfig with library defaults (sample rate, volume, internal buffer).
 * @param[out] pConfig  Must not be NULL.
 */
void TIREMO_MIC_App_GetDefaultConfig(TiremoMicAppConfig_t *pConfig);

/**
 * @brief Init using TIREMO_MIC_App_GetDefaultConfig() — no cfg struct in user code.
 * @return  true on success.
 */
bool TIREMO_MIC_App_InitDefault(void);

/**
 * @brief   GPIO + sample rate + buffer setup. Does not start capture.
 * @return  true if configuration is valid and BSP rate was set.
 */
bool TIREMO_MIC_App_Init(const TiremoMicAppConfig_t *pConfig);

/** @brief Start BSP capture after successful Init. */
bool TIREMO_MIC_App_Start(void);

/** @brief Stop capture; state returns to READY. */
void TIREMO_MIC_App_Stop(void);

TiremoMicState_e TIREMO_MIC_App_GetState(void);

/** @brief Monotonic sample counter since last Start (wraps at UINT32_MAX). */
uint32_t TIREMO_MIC_App_GetSampleCount(void);

uint32_t TIREMO_MIC_App_GetSampleRate(void);

/** @brief Set by APP when a full buffer completes. */
bool TIREMO_MIC_App_IsBufferReady(void);

void TIREMO_MIC_App_ClearBufferReady(void);

/**
 * @brief       High-pass filter step (also usable standalone).
 * @param[in]   rawSample  12-bit ADC value.
 * @param[in]   pFilter    Persistent filter state.
 * @return      Signed filtered sample before PCM offset.
 */
int16_t TIREMO_MIC_App_ApplyHpFilter(int32_t rawSample, TiremoMicHpFilter_t *pFilter);

/** @brief Pointer to the circular capture buffer. */
const uint16_t *TIREMO_MIC_App_GetBuffer(void);

/**
 * @brief       Analyze one PCM buffer; update level and voice state.
 * @param[in]   pBuffer  PCM samples centred at TIREMO_MIC_PCM_CENTER.
 * @param[in]   size     Sample count.
 * @return      Alert when voice state changes; TIREMO_MIC_ALERT_NONE otherwise.
 */
TiremoMicVoiceResult_t TIREMO_MIC_App_ProcessVoice(const uint16_t *pBuffer, uint32_t size);

/** @brief Smoothed RMS level from the last ProcessVoice() call. */
uint32_t TIREMO_MIC_App_GetVoiceLevel(void);

/**
 * @brief       Short human-readable label for a voice alert.
 * @param[in]   alert  Alert type (not TIREMO_MIC_ALERT_NONE).
 */
const char *TIREMO_MIC_App_GetAlertLabel(TiremoMicAlert_e alert);

/**
 * @brief       Called when voice mod changes (HIGH / NORMAL / SILENCE).
 * @details     Weak stub — override in prv_user_code.c for UART, LED, buzzer.
 *              ProcessVoice() already ran in APP; do not call it again here.
 * @param[in]   result  .alert != NONE, .level = smoothed RMS.
 */
void TIREMO_MIC_App_OnVoiceAlert(TiremoMicVoiceResult_t result);

#if (TIREMO_MIC_APP_DEBUG_RMS_UART == 1)
/**
 * @brief       Periodic RMS debug hook when TIREMO_MIC_APP_DEBUG_RMS_UART is 1.
 * @details     Weak stub — override in prv_user_code.c to print level on UART.
 */
void TIREMO_MIC_App_OnVoiceLevelDebug(uint32_t level);
#endif

#endif /* TIREMO_MIC_APP_H_ */
