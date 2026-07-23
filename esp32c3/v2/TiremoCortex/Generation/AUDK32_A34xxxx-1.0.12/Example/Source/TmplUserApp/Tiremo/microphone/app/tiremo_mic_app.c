/**
 *******************************************************************************
 * @file        tiremo_mic_app.c
 * @brief       Tiremo microphone application layer implementation
 *
 * @details     User application: voice detection (HIGH / NORMAL / SILENCE).
 *              Buffer dolunca ProcessVoice() burada otomatik çalışır.
 *              Çıkış için weak OnVoiceAlert() — isteğe bağlı prv'de UART/LED.
 *
 * Copyright 2024 Tiremo. All rights reserved.
 ******************************************************************************/

#include "abov_config.h"
#include "abov_module_config.h"
#include "tiremo_mic_app.h"

#include "../bsp/tiremo_mic_bsp.h"

#define TIREMO_MIC_HP_COEFF           (0xF8)
#define TIREMO_MIC_HP_SHIFT           (256)

#define TIREMO_MIC_CLAMP(v, lo, hi) \
    (((v) < (lo)) ? (lo) : (((v) > (hi)) ? (hi) : (v)))

/* --- Module state --------------------------------------------------------- */

static uint16_t s_captureBuffer[TIREMO_MIC_APP_BUFFER_SIZE];

static TiremoMicState_e s_state = TIREMO_MIC_STATE_RESET;
static uint32_t s_sampleRateHz = TIREMO_MIC_DEFAULT_SAMPLE_RATE;
static uint32_t s_volume = TIREMO_MIC_DEFAULT_VOLUME;
static uint16_t *s_pBuffer = NULL;
static uint32_t s_bufferSize = 0U;
static uint32_t s_writeIndex = 0U;
static uint32_t s_sampleCount = 0U;
static volatile bool s_bufferReady = false;
static TiremoMicHpFilter_t s_hpFilter;

typedef enum
{
    APP_VOICE_NORMAL = 0,
    APP_VOICE_HIGH,
    APP_VOICE_SILENCE
} AppVoiceState_e;

static AppVoiceState_e s_voiceState = APP_VOICE_NORMAL;
static uint32_t s_loudStreak = 0U;
static uint32_t s_quietStreak = 0U;
static uint32_t s_voiceLevel = 0U;
static uint32_t s_voiceBufCount = 0U;

static void TIREMO_MIC_App_HandleDmaBlock(const uint16_t *pRawSamples, uint32_t sampleCount);
static void TIREMO_MIC_App_HandleSample(uint16_t rawSample);
static void TIREMO_MIC_App_RegisterBspHandlers(void);

static bool TIREMO_MIC_App_ValidateConfig(const TiremoMicAppConfig_t *pConfig)
{
    if (pConfig == NULL)
    {
        return false;
    }

    if ((pConfig->pBuffer == NULL) || (pConfig->bufferSize == 0U))
    {
        return false;
    }

    if ((pConfig->sampleRateHz == 0U) ||
        (pConfig->sampleRateHz > TIREMO_MIC_APP_MAX_SAMPLE_RATE))
    {
        return false;
    }

    if (pConfig->volume > 100U)
    {
        return false;
    }

    return true;
}

/* --- Sample path: BSP → buffer → voice analysis --------------------------- */

static void TIREMO_MIC_App_HandleBufferReady(uint16_t *pBuffer, uint32_t size)
{
    TiremoMicVoiceResult_t result;

    result = TIREMO_MIC_App_ProcessVoice(pBuffer, size);
    s_voiceBufCount++;

    if (result.alert != TIREMO_MIC_ALERT_NONE)
    {
        TIREMO_MIC_App_OnVoiceAlert(result);
    }
#if (TIREMO_MIC_APP_DEBUG_RMS_UART == 1)
    else if ((s_voiceBufCount % TIREMO_MIC_APP_DEBUG_RMS_EVERY_N) == 0U)
    {
        TIREMO_MIC_App_OnVoiceLevelDebug(result.level);
    }
#endif
}

static void TIREMO_MIC_App_StoreSample(uint16_t rawAdc)
{
    uint16_t raw12 = TIREMO_MIC_ADC_CLAMP12(rawAdc);

#if (TIREMO_MIC_APP_STORE_RAW_ADC == 1)
    s_pBuffer[s_writeIndex] = raw12;
#else
    int16_t filtered;

    filtered = TIREMO_MIC_App_ApplyHpFilter((int32_t)raw12, &s_hpFilter);
    s_pBuffer[s_writeIndex] = (uint16_t)((int32_t)filtered + 32768);
#endif

    s_writeIndex++;

    if (s_writeIndex >= s_bufferSize)
    {
        s_writeIndex = 0U;
        s_bufferReady = true;
        TIREMO_MIC_App_HandleBufferReady(s_pBuffer, s_bufferSize);
    }

    s_sampleCount++;
}

const uint16_t *TIREMO_MIC_App_GetBuffer(void)
{
    return s_pBuffer;
}

/* --- Voice level analysis (high / normal / silence) ----------------------- */

static void TIREMO_MIC_App_ResetVoiceState(void)
{
    s_voiceState = APP_VOICE_NORMAL;
    s_loudStreak = 0U;
    s_quietStreak = 0U;
    s_voiceLevel = 0U;
}

static uint32_t TIREMO_MIC_App_Isqrt64(uint64_t x)
{
    uint64_t op = x;
    uint64_t res = 0U;
    uint64_t one = 1ULL << 62;

    if (x == 0U)
    {
        return 0U;
    }

    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0U)
    {
        if (op >= (res + one))
        {
            op -= (res + one);
            res = (res >> 1) + one;
        }
        else
        {
            res >>= 1;
        }
        one >>= 2;
    }

    return (uint32_t)res;
}

static uint32_t TIREMO_MIC_App_BufferRms(const uint16_t *pBuffer, uint32_t size)
{
    uint32_t i;
    uint64_t sumSq = 0U;

    if ((pBuffer == NULL) || (size == 0U))
    {
        return 0U;
    }

    for (i = 0U; i < size; i++)
    {
        int32_t x = (int32_t)pBuffer[i] - (int32_t)TIREMO_MIC_PCM_CENTER;
        int64_t sq = (int64_t)x * (int64_t)x;

        sumSq += (uint64_t)sq;
    }

    return TIREMO_MIC_App_Isqrt64(sumSq / (uint64_t)size);
}

static uint32_t TIREMO_MIC_App_UpdateVoiceLevel(uint32_t rms)
{
    if (rms > s_voiceLevel)
    {
        uint32_t rise = (rms - s_voiceLevel) / TIREMO_MIC_VOICE_LEVEL_ATTACK_DIV;

        if (rise == 0U)
        {
            rise = 1U;
        }
        s_voiceLevel += rise;
        if (s_voiceLevel > rms)
        {
            s_voiceLevel = rms;
        }
    }
    else if (s_voiceLevel > rms)
    {
        uint32_t fall = (s_voiceLevel - rms) / TIREMO_MIC_VOICE_LEVEL_RELEASE_DIV;

        if (fall == 0U)
        {
            fall = 1U;
        }
        if (fall > s_voiceLevel)
        {
            s_voiceLevel = 0U;
        }
        else
        {
            s_voiceLevel -= fall;
        }
    }

    return s_voiceLevel;
}

static TiremoMicAlert_e TIREMO_MIC_App_StateToAlert(AppVoiceState_e state)
{
    switch (state)
    {
    case APP_VOICE_HIGH:
        return TIREMO_MIC_ALERT_HIGH_VOICE;

    case APP_VOICE_SILENCE:
        return TIREMO_MIC_ALERT_SILENCE;

    case APP_VOICE_NORMAL:
    default:
        return TIREMO_MIC_ALERT_NORMAL;
    }
}

static TiremoMicAlert_e TIREMO_MIC_App_UpdateVoiceState(uint32_t level)
{
    switch (s_voiceState)
    {
    case APP_VOICE_HIGH:
        if (level >= TIREMO_MIC_VOICE_LOUD_EXIT)
        {
            s_quietStreak = 0U;
            return TIREMO_MIC_ALERT_NONE;
        }

        s_quietStreak++;
        if (s_quietStreak < TIREMO_MIC_VOICE_NORMAL_BUFFERS)
        {
            return TIREMO_MIC_ALERT_NONE;
        }

        s_quietStreak = 0U;
        if (level <= TIREMO_MIC_VOICE_SILENCE_ENTER)
        {
            s_voiceState = APP_VOICE_SILENCE;
        }
        else
        {
            s_voiceState = APP_VOICE_NORMAL;
        }
        return TIREMO_MIC_App_StateToAlert(s_voiceState);

    case APP_VOICE_SILENCE:
        if (level <= TIREMO_MIC_VOICE_SILENCE_ENTER)
        {
            s_loudStreak = 0U;
            return TIREMO_MIC_ALERT_NONE;
        }

        if (level >= TIREMO_MIC_VOICE_LOUD_ENTER)
        {
            s_loudStreak++;
            if (s_loudStreak >= TIREMO_MIC_VOICE_LOUD_BUFFERS)
            {
                s_loudStreak = 0U;
                s_voiceState = APP_VOICE_HIGH;
                return TIREMO_MIC_ALERT_HIGH_VOICE;
            }
            return TIREMO_MIC_ALERT_NONE;
        }

        if (level > TIREMO_MIC_VOICE_SILENCE_EXIT)
        {
            s_loudStreak = 0U;
            s_voiceState = APP_VOICE_NORMAL;
            return TIREMO_MIC_ALERT_NORMAL;
        }
        return TIREMO_MIC_ALERT_NONE;

    case APP_VOICE_NORMAL:
    default:
        if (level >= TIREMO_MIC_VOICE_LOUD_ENTER)
        {
            s_loudStreak++;
            s_quietStreak = 0U;

            if (s_loudStreak >= TIREMO_MIC_VOICE_LOUD_BUFFERS)
            {
                s_loudStreak = 0U;
                s_voiceState = APP_VOICE_HIGH;
                return TIREMO_MIC_ALERT_HIGH_VOICE;
            }
        }
        else if (level <= TIREMO_MIC_VOICE_SILENCE_ENTER)
        {
            s_loudStreak = 0U;
            s_quietStreak++;

            if (s_quietStreak >= TIREMO_MIC_VOICE_NORMAL_BUFFERS)
            {
                s_quietStreak = 0U;
                s_voiceState = APP_VOICE_SILENCE;
                return TIREMO_MIC_ALERT_SILENCE;
            }
        }
        else
        {
            s_loudStreak = 0U;
            s_quietStreak = 0U;
        }
        return TIREMO_MIC_ALERT_NONE;
    }
}

TiremoMicVoiceResult_t TIREMO_MIC_App_ProcessVoice(const uint16_t *pBuffer, uint32_t size)
{
    TiremoMicVoiceResult_t result =
    {
        .alert = TIREMO_MIC_ALERT_NONE,
        .level = 0U
    };
    uint32_t rms;

    if ((pBuffer == NULL) || (size == 0U))
    {
        return result;
    }

    rms = TIREMO_MIC_App_BufferRms(pBuffer, size);
    result.level = TIREMO_MIC_App_UpdateVoiceLevel(rms);
    result.alert = TIREMO_MIC_App_UpdateVoiceState(result.level);

    return result;
}

uint32_t TIREMO_MIC_App_GetVoiceLevel(void)
{
    return s_voiceLevel;
}

const char *TIREMO_MIC_App_GetAlertLabel(TiremoMicAlert_e alert)
{
    switch (alert)
    {
    case TIREMO_MIC_ALERT_HIGH_VOICE:
        return "*** HIGH VOICE ***";

    case TIREMO_MIC_ALERT_SILENCE:
        return "--- SILENCE ---";

    case TIREMO_MIC_ALERT_NORMAL:
        return "--- NORMAL ---";

    case TIREMO_MIC_ALERT_NONE:
    default:
        return "";
    }
}

/* --- Public API ----------------------------------------------------------- */

void TIREMO_MIC_App_GetDefaultConfig(TiremoMicAppConfig_t *pConfig)
{
    if (pConfig == NULL)
    {
        return;
    }

    pConfig->sampleRateHz = TIREMO_MIC_DEFAULT_SAMPLE_RATE;
    pConfig->volume = TIREMO_MIC_DEFAULT_VOLUME;
    pConfig->pBuffer = s_captureBuffer;
    pConfig->bufferSize = TIREMO_MIC_APP_BUFFER_SIZE;
}

bool TIREMO_MIC_App_InitDefault(void)
{
    TiremoMicAppConfig_t cfg;

    TIREMO_MIC_App_GetDefaultConfig(&cfg);
    return TIREMO_MIC_App_Init(&cfg);
}

bool TIREMO_MIC_App_Init(const TiremoMicAppConfig_t *pConfig)
{
    if (!TIREMO_MIC_App_ValidateConfig(pConfig))
    {
        return false;
    }

    if (s_state == TIREMO_MIC_STATE_RECORDING)
    {
        return false;
    }

    TIREMO_MIC_Init();
    TIREMO_MIC_App_RegisterBspHandlers();

    s_sampleRateHz = pConfig->sampleRateHz;
    s_volume = pConfig->volume;
    s_pBuffer = pConfig->pBuffer;
    s_bufferSize = pConfig->bufferSize;
    s_writeIndex = 0U;
    s_sampleCount = 0U;
    s_bufferReady = false;
    s_hpFilter.z = 0;
    s_hpFilter.oldOut = 0;
    s_hpFilter.oldIn = 0;
    TIREMO_MIC_App_ResetVoiceState();
    s_voiceBufCount = 0U;

    if (!TIREMO_MIC_BSP_SetSampleRate(s_sampleRateHz))
    {
        s_state = TIREMO_MIC_STATE_ERROR;
        return false;
    }

    s_state = TIREMO_MIC_STATE_READY;
    return true;
}

bool TIREMO_MIC_App_Start(void)
{
    if (s_state != TIREMO_MIC_STATE_READY)
    {
        return false;
    }

    s_writeIndex = 0U;
    s_sampleCount = 0U;
    s_bufferReady = false;
    s_hpFilter.z = 0;
    s_hpFilter.oldOut = 0;
    s_hpFilter.oldIn = 0;
    TIREMO_MIC_App_ResetVoiceState();
    s_voiceBufCount = 0U;

    s_state = TIREMO_MIC_STATE_RECORDING;

    if (!TIREMO_MIC_BSP_Start())
    {
        s_state = TIREMO_MIC_STATE_ERROR;
        return false;
    }

    return true;
}

void TIREMO_MIC_App_Stop(void)
{
    if (s_state != TIREMO_MIC_STATE_RECORDING)
    {
        return;
    }

    TIREMO_MIC_BSP_Stop();
    s_state = TIREMO_MIC_STATE_READY;
}

TiremoMicState_e TIREMO_MIC_App_GetState(void)
{
    return s_state;
}

uint32_t TIREMO_MIC_App_GetSampleCount(void)
{
    return s_sampleCount;
}

uint32_t TIREMO_MIC_App_GetSampleRate(void)
{
    return s_sampleRateHz;
}

bool TIREMO_MIC_App_IsBufferReady(void)
{
    return s_bufferReady;
}

void TIREMO_MIC_App_ClearBufferReady(void)
{
    s_bufferReady = false;
}

int16_t TIREMO_MIC_App_ApplyHpFilter(int32_t rawSample, TiremoMicHpFilter_t *pFilter)
{
    int32_t out;
    int32_t signedSample;

    signedSample = rawSample - (int32_t)TIREMO_MIC_ADC_OFFSET;
    pFilter->z = (signedSample * (int32_t)s_volume) / 100;
    pFilter->z *= (int32_t)TIREMO_MIC_SIGNAL_GAIN;

    pFilter->oldOut = (TIREMO_MIC_HP_COEFF *
                       (pFilter->oldOut + pFilter->z - pFilter->oldIn)) /
                      TIREMO_MIC_HP_SHIFT;
    pFilter->oldIn = pFilter->z;

    out = TIREMO_MIC_CLAMP(pFilter->oldOut, -32760, 32760);
    return (int16_t)out;
}

/* --- BSP data handlers (registered from APP Init) ------------------------- */

static void TIREMO_MIC_App_HandleDmaBlock(const uint16_t *pRawSamples, uint32_t sampleCount)
{
    uint32_t i;

    if ((s_state != TIREMO_MIC_STATE_RECORDING) || (pRawSamples == NULL))
    {
        return;
    }

    for (i = 0U; i < sampleCount; i++)
    {
        TIREMO_MIC_App_StoreSample(TIREMO_MIC_ADC_RAW_FROM_DDR(pRawSamples[i]));
    }
}

static void TIREMO_MIC_App_HandleSample(uint16_t rawSample)
{
    if (s_state != TIREMO_MIC_STATE_RECORDING)
    {
        return;
    }

    TIREMO_MIC_App_StoreSample(rawSample);
}

static void TIREMO_MIC_App_RegisterBspHandlers(void)
{
    static const TiremoMicBspDataHandlers_t s_handlers =
    {
        TIREMO_MIC_App_HandleDmaBlock,
        TIREMO_MIC_App_HandleSample
    };

    TIREMO_MIC_BSP_SetDataHandlers(&s_handlers);
}

__attribute__((weak)) void TIREMO_MIC_App_OnVoiceAlert(TiremoMicVoiceResult_t result)
{
    (void)result;
}

#if (TIREMO_MIC_APP_DEBUG_RMS_UART == 1)
__attribute__((weak)) void TIREMO_MIC_App_OnVoiceLevelDebug(uint32_t level)
{
    (void)level;
}
#endif
