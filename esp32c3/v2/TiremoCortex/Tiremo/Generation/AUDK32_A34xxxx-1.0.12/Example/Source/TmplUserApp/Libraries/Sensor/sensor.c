#include "sensor.h"
#include "sensor_alarm.h"
#include "../../config/board_config.h"
#include "../DebugLibrary/debug_framework.h"
#include "../SHT40/SHT40_app.h"
#include "../BatteryReading/battery_reading.h"
#include "../LISDE12TR/LIS2DE12_app.h"
#include "../MP23ABS1/mp23abs1_sensor.h"
#include "hal_pcu.h"
#include "abov_config.h"
#include <string.h>
#include <stdio.h>

static void s_led_on(PCU_ID_e port, PCU_PIN_ID_e pin)  { HAL_PCU_SetOutputBit(port, pin, PCU_OUTPUT_BIT_CLEAR); }
static void s_led_off(PCU_ID_e port, PCU_PIN_ID_e pin) { HAL_PCU_SetOutputBit(port, pin, PCU_OUTPUT_BIT_SET); }

/* Internal sensor data */
static SensorData_t s_data;

/* Microphone buffer */
#define AUDIO_BUFFER_SIZE   1024
#define AUDIO_SAMPLE_RATE   16000
#define AUDIO_VOLUME        80

static uint16_t s_AudioBuffer[AUDIO_BUFFER_SIZE];
static volatile uint8_t s_bufferReady = 0;

/* LED helpers from prv_user_code.c */
extern void SYSTICK_Wait(uint32_t un32TimeMS);

/*-------------------------------------------------------------------*/
/* Microphone callbacks                                               */
/*-------------------------------------------------------------------*/
void MP23ABS1_BufferReady_Callback(uint16_t *pBuffer, uint32_t Size)
{
    s_bufferReady = 1;
}

void MP23ABS1_Error_Callback(void)
{
    DebugFramework_Puts("ERROR: MP23ABS1 Microphone Error!\n\r");
}

/*-------------------------------------------------------------------*/
/* Compute RMS from audio buffer                                      */
/*-------------------------------------------------------------------*/
static uint32_t Compute_RMS(const uint16_t *buf, uint32_t len)
{
    if (len == 0) return 0;

    uint64_t sumSq = 0;
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        /* Samples are unsigned 16-bit centered at 32768. Convert to signed. */
        int32_t sample = (int32_t)buf[i] - 32768;
        sumSq += (uint64_t)((int64_t)sample * sample);
    }
    /* Integer square root approximation */
    uint64_t mean = sumSq / len;
    uint32_t rms = 0;
    if (mean > 0)
    {
        /* Newton's method for integer sqrt */
        uint64_t x = mean;
        uint64_t y = (x + 1) / 2;
        while (y < x) { x = y; y = (x + mean / x) / 2; }
        rms = (uint32_t)x;
    }
    return rms;
}

/*-------------------------------------------------------------------*/
void Sensor_LEDTest(void)
{
    typedef struct { PCU_ID_e port; PCU_PIN_ID_e pin; } LedEntry;
    LedEntry ledArray[] = {
        {BOARD_LED1_PORT,  BOARD_LED1_PIN},
        {BOARD_LED2_PORT,  BOARD_LED2_PIN},
        {BOARD_LED3_PORT,  BOARD_LED3_PIN},
        {BOARD_LED4_PORT,  BOARD_LED4_PIN},
        {BOARD_LED5_PORT,  BOARD_LED5_PIN},
        {BOARD_LED6_PORT,  BOARD_LED6_PIN},
        {BOARD_LED7_PORT,  BOARD_LED7_PIN},
        {BOARD_LED8_PORT,  BOARD_LED8_PIN},
        {BOARD_LED9_PORT,  BOARD_LED9_PIN},
        {BOARD_LED10_PORT, BOARD_LED10_PIN},
    };
    uint32_t i;

    DebugFramework_PutsLine("\n\r========================================");
    DebugFramework_PutsLine("         LED HARDWARE TEST");
    DebugFramework_PutsLine("========================================\n\r");

    for (i = 0; i < 10; i++)
    {
        s_led_on(ledArray[i].port, ledArray[i].pin);
        SYSTICK_Wait(200);
    }

    SYSTICK_Wait(500);
    for (i = 0; i < 10; i++)
    {
        s_led_off(ledArray[i].port, ledArray[i].pin);
        SYSTICK_Wait(100);
    }

    DebugFramework_PutsLine("[OK] LED Test Complete\n\r");
}

/*-------------------------------------------------------------------*/
uint8_t Sensor_TestAll(void)
{
    uint8_t passed = 0;
    uint8_t failed = 0;

    DebugFramework_PutsLine("\n\r========================================");
    DebugFramework_PutsLine("       SENSOR INITIALIZATION TEST");
    DebugFramework_PutsLine("========================================\n\r");

    /* SHT40 */
    DebugFramework_PutsLine("[TEST] SHT40 Temperature & Humidity...");
    if (SHT40_Init() == 0) {
        DebugFramework_PutsLine("[PASS] SHT40 OK");
        passed++;
    } else {
        DebugFramework_PutsLine("[FAIL] SHT40 init failed");
        failed++;
    }

    /* Battery */
    DebugFramework_PutsLine("[TEST] Battery Reading...");
    if (BatteryReading_Init() == 0) {
        DebugFramework_PutsLine("[PASS] Battery OK");
        passed++;
    } else {
        DebugFramework_PutsLine("[FAIL] Battery init failed");
        failed++;
    }

    /* LIS2DE12 */
    DebugFramework_PutsLine("[TEST] LIS2DE12 Accelerometer...");
    if (LIS2DE12_Init() == 0) {
        DebugFramework_PutsLine("[PASS] LIS2DE12 OK");
        passed++;
    } else {
        DebugFramework_PutsLine("[FAIL] LIS2DE12 init failed");
        failed++;
    }

    /* MP23ABS1 Microphone */
    DebugFramework_PutsLine("[TEST] MP23ABS1 Microphone...");
    MP23ABS1_Init_t micInit;
    micInit.SampleRate = AUDIO_SAMPLE_RATE;
    micInit.Volume     = AUDIO_VOLUME;
    micInit.pBuffer    = s_AudioBuffer;
    micInit.BufferSize = AUDIO_BUFFER_SIZE;

    if (MP23ABS1_Init(&micInit) == MP23ABS1_OK) {
        DebugFramework_PutsLine("[PASS] MP23ABS1 OK");
        passed++;
    } else {
        DebugFramework_PutsLine("[FAIL] MP23ABS1 init failed");
        failed++;
    }

    DebugFramework_PutsLine("----------------------------------------");
    DebugFramework_Printf("Passed: %u  Failed: %u  Total: %u\n\r",
                           passed, failed, passed + failed);
    if (failed == 0)
        DebugFramework_PutsLine("*** ALL SENSORS OK ***\n\r");
    else
        DebugFramework_Printf("!!! %u SENSOR(S) FAILED !!!\n\r", failed);
    DebugFramework_PutsLine("========================================\n\r");

    return failed;
}

/*-------------------------------------------------------------------*/
SensorData_t* Sensor_ReadOnly(void)
{
    memset(&s_data, 0, sizeof(s_data));

    /* SHT40 */
    if (SHT40_ReadSensor() == 0) {
        s_data.sht40_ok       = 1;
        s_data.temperature_mC = sht40_temperature;
        s_data.humidity_mRH   = sht40_humidity;
    }

    /* Battery */
    {
        uint16_t vcoreRaw = 0;
        uint32_t avddMv   = 0;
        if (BatteryReading_ReadSupplyVoltage(&vcoreRaw, &avddMv) == 0) {
            s_data.battery_ok = 1;
            s_data.battery_mV = avddMv;
        }
    }

    /* LIS2DE12 */
    if (LIS2DE12_ReadAcceleration() == 0) {
        s_data.lis2de12_ok = 1;
        s_data.accel_x_mg  = lis2de12_accel_x;
        s_data.accel_y_mg  = lis2de12_accel_y;
        s_data.accel_z_mg  = lis2de12_accel_z;
    }

    /* MP23ABS1 Microphone */
    if (MP23ABS1_StartRecording() == MP23ABS1_OK) {
        SYSTICK_Wait(1000);
        MP23ABS1_StopRecording();
        uint32_t count = MP23ABS1_GetSampleCount();
        if (count > 0) {
            uint32_t calcLen = (count < AUDIO_BUFFER_SIZE) ? count : AUDIO_BUFFER_SIZE;
            s_data.mic_ok  = 1;
            s_data.mic_rms = Compute_RMS(s_AudioBuffer, calcLen);
        }
    }

    return &s_data;
}

/*-------------------------------------------------------------------*/
SensorData_t* Sensor_ReadAndPrint(void)
{
    memset(&s_data, 0, sizeof(s_data));

    DebugFramework_PutsLine("--- Sensor Readings ---");

    /* SHT40 */
    if (SHT40_ReadSensor() == 0) {
        s_data.sht40_ok = 1;
        s_data.temperature_mC = sht40_temperature;
        s_data.humidity_mRH   = sht40_humidity;
    } else {
        DebugFramework_PutsLine("[SHT40] Read FAILED");
    }

    /* Battery */
    {
        uint16_t vcoreRaw = 0;
        uint32_t avddMv   = 0;
        if (BatteryReading_ReadSupplyVoltage(&vcoreRaw, &avddMv) == 0) {
            s_data.battery_ok = 1;
            s_data.battery_mV = avddMv;
        } else {
            DebugFramework_PutsLine("[BATT]  Read FAILED");
        }
    }

    /* LIS2DE12 */
    if (LIS2DE12_ReadAcceleration() == 0) {
        s_data.lis2de12_ok = 1;
        s_data.accel_x_mg = lis2de12_accel_x;
        s_data.accel_y_mg = lis2de12_accel_y;
        s_data.accel_z_mg = lis2de12_accel_z;
    } else {
        DebugFramework_PutsLine("[ACCEL] Read FAILED");
    }

    /* MP23ABS1 Microphone - fixed 1 second recording, then compute RMS */
    if (MP23ABS1_StartRecording() == MP23ABS1_OK) {
        SYSTICK_Wait(1000);  
        MP23ABS1_StopRecording();

        uint32_t count = MP23ABS1_GetSampleCount();
        if (count > 0) {
            uint32_t calcLen = (count < AUDIO_BUFFER_SIZE) ? count : AUDIO_BUFFER_SIZE;
            s_data.mic_ok  = 1;
            s_data.mic_rms = Compute_RMS(s_AudioBuffer, calcLen);
        } else {
            DebugFramework_PutsLine("[MIC]   No samples captured");
        }
    } else {
        DebugFramework_PutsLine("[MIC]   Start FAILED");
    }
    DebugFramework_Printf("[SHT40] Temp: %ld.%03lu C  Hum: %ld.%03lu %%\n\r",
    (long)(s_data.temperature_mC / 1000),
    (unsigned long)(s_data.temperature_mC >= 0 ? s_data.temperature_mC % 1000 : (-s_data.temperature_mC) % 1000),
    (long)(s_data.humidity_mRH / 1000),
    (unsigned long)(s_data.humidity_mRH % 1000));
    DebugFramework_Printf("[BATT]  Voltage: %lu.%03lu V\n\r",
    (unsigned long)(s_data.battery_mV / 1000), (unsigned long)(s_data.battery_mV % 1000));
    DebugFramework_Printf("[ACCEL] X:%d  Y:%d  Z:%d mg\n\r",
    s_data.accel_x_mg, s_data.accel_y_mg, s_data.accel_z_mg);
    DebugFramework_Printf("[MIC]   RMS: %lu\n\r", (unsigned long)s_data.mic_rms);

    return &s_data;
}

/*-------------------------------------------------------------------*/
uint16_t Sensor_FormatJSON(const SensorData_t *pData, char *outBuf, uint16_t bufLen)
{
    /* Avoid float format specifiers (%.2f etc.) — they hang on this MCU.
     * Use integer arithmetic instead, matching Sensor_ReadAndPrint style. */
    long  temp_int  = (long)(pData->temperature_mC / 1000);
    unsigned long temp_frac = (unsigned long)(pData->temperature_mC >= 0
                                ? (pData->temperature_mC % 1000)
                                : ((-pData->temperature_mC) % 1000));

    long  hum_int   = (long)(pData->humidity_mRH / 1000);
    unsigned long hum_frac  = (unsigned long)(pData->humidity_mRH % 1000);

    unsigned long bat_int   = (unsigned long)(pData->battery_mV / 1000);
    unsigned long bat_frac  = (unsigned long)(pData->battery_mV % 1000);

    int n = snprintf(outBuf, bufLen,
        "{\"temp\":%ld.%02lu,\"hum\":%ld.%02lu,\"bat\":%lu.%03lu,"
        "\"ax\":%d,\"ay\":%d,\"az\":%d,\"mic_rms\":%lu}",
        temp_int, temp_frac / 10,
        hum_int,  hum_frac  / 10,
        bat_int,  bat_frac,
        (int)pData->accel_x_mg,
        (int)pData->accel_y_mg,
        (int)pData->accel_z_mg,
        (unsigned long)pData->mic_rms);

    if (n < 0 || n >= (int)bufLen) n = bufLen - 1;
    return (uint16_t)n;
}

/*-------------------------------------------------------------------*/
/* Alarm detection (linked via sensor.c — no separate build entry)    */
/*-------------------------------------------------------------------*/

static uint8_t s_tempAbove = 0U;
static uint8_t s_fallActive = 0U;
static uint8_t s_soundAbove = 0U;

uint8_t Sensor_PollAlarms(const SensorData_t *pData,
                          SensorAlarmType_t *pOut,
                          uint8_t maxOut)
{
    uint8_t count = 0U;

    if (pData == NULL || pOut == NULL || maxOut == 0U)
        return 0U;

    if (pData->sht40_ok)
    {
        if (pData->temperature_mC > SENSOR_ALARM_TEMP_THRESHOLD_MC)
        {
            if (!s_tempAbove)
            {
                s_tempAbove = 1U;
                if (count < maxOut)
                    pOut[count++] = SENSOR_ALARM_TEMP_HIGH;
            }
        }
        else if (s_tempAbove)
        {
            s_tempAbove = 0U;
            if (count < maxOut)
                pOut[count++] = SENSOR_ALARM_TEMP_NORMAL;
        }
    }

    if (pData->lis2de12_ok)
    {
        /* Only downward: Z <= 1000 - 300 mg */
        if (pData->accel_z_mg <= SENSOR_ALARM_FALL_Z_MAX_MG)
        {
            if (!s_fallActive)
            {
                s_fallActive = 1U;
                if (count < maxOut)
                    pOut[count++] = SENSOR_ALARM_FALL;
            }
        }
        else if (s_fallActive)
        {
            s_fallActive = 0U;
            if (count < maxOut)
                pOut[count++] = SENSOR_ALARM_FALL_NORMAL;
        }
    }

    if (pData->mic_ok)
    {
        if (pData->mic_rms > SENSOR_ALARM_MIC_RMS_THRESHOLD)
        {
            if (!s_soundAbove)
            {
                s_soundAbove = 1U;
                if (count < maxOut)
                    pOut[count++] = SENSOR_ALARM_LOUD_SOUND;
            }
        }
        else if (s_soundAbove)
        {
            s_soundAbove = 0U;
            if (count < maxOut)
                pOut[count++] = SENSOR_ALARM_SOUND_NORMAL;
        }
    }

    return count;
}

uint16_t Sensor_FormatAlarmJSON(SensorAlarmType_t alarm,
                                const SensorData_t *pData,
                                char *outBuf,
                                uint16_t bufLen)
{
    int n;

    if (pData == NULL || outBuf == NULL || bufLen == 0U)
        return 0U;

    switch (alarm)
    {
    case SENSOR_ALARM_TEMP_HIGH:
    {
        long temp_int = (long)(pData->temperature_mC / 1000);
        unsigned long temp_frac = (unsigned long)(
            pData->temperature_mC >= 0
                ? (pData->temperature_mC % 1000)
                : ((-pData->temperature_mC) % 1000));
        n = snprintf(outBuf, bufLen,
                     "{\"alarmType\":\"TEMPERATURE_HIGH\",\"severity\":\"MAJOR\","
                     "\"code\":\"Temp_HIGH\",\"exception\":\"temp: %ld.%02lu\"}",
                     temp_int, temp_frac / 10U);
        break;
    }

    case SENSOR_ALARM_TEMP_NORMAL:
    {
        long temp_int = (long)(pData->temperature_mC / 1000);
        unsigned long temp_frac = (unsigned long)(
            pData->temperature_mC >= 0
                ? (pData->temperature_mC % 1000)
                : ((-pData->temperature_mC) % 1000));
        n = snprintf(outBuf, bufLen,
                     "{\"alarmType\":\"TEMPERATURE_NORMAL\",\"severity\":\"INFO\","
                     "\"code\":\"Temp_NORMAL\",\"exception\":\"temp: %ld.%02lu\"}",
                     temp_int, temp_frac / 10U);
        break;
    }

    case SENSOR_ALARM_FALL:
        n = snprintf(outBuf, bufLen,
                     "{\"alarmType\":\"FALL_DETECTED\",\"severity\":\"MAJOR\","
                     "\"code\":\"Fall_DETECTED\",\"exception\":\"az: %d\"}",
                     (int)pData->accel_z_mg);
        break;

    case SENSOR_ALARM_FALL_NORMAL:
        n = snprintf(outBuf, bufLen,
                     "{\"alarmType\":\"FALL_NORMAL\",\"severity\":\"INFO\","
                     "\"code\":\"Fall_NORMAL\",\"exception\":\"az: %d\"}",
                     (int)pData->accel_z_mg);
        break;

    case SENSOR_ALARM_LOUD_SOUND:
        n = snprintf(outBuf, bufLen,
                     "{\"alarmType\":\"LOUD_SOUND\",\"severity\":\"MAJOR\","
                     "\"code\":\"Sound_HIGH\",\"exception\":\"mic_rms: %lu\"}",
                     (unsigned long)pData->mic_rms);
        break;

    case SENSOR_ALARM_SOUND_NORMAL:
        n = snprintf(outBuf, bufLen,
                     "{\"alarmType\":\"SOUND_NORMAL\",\"severity\":\"INFO\","
                     "\"code\":\"Sound_NORMAL\",\"exception\":\"mic_rms: %lu\"}",
                     (unsigned long)pData->mic_rms);
        break;

    default:
        return 0U;
    }

    if (n < 0 || n >= (int)bufLen)
        return 0U;

    return (uint16_t)n;
}
