/**
 *******************************************************************************
 * @file        sensor.c
 * @brief       Multi-sensor facade for TiremoCortex (SHT40 / LIS2DE12 / mic / battery)
 ******************************************************************************/

#include "sensor.h"
#include "sensor_alarm.h"

#include "../Tiremo/DebugLibrary/debug_framework.h"
#include "../Tiremo/common/tiremo_systick.h"
#include "../Tiremo/led/tiremo_led.h"
#include "../Tiremo/sht40/tiremo_sht40.h"
#include "../Tiremo/battery/tiremo_battery.h"
#include "../Tiremo/lis2de12tr/tiremo_lis2de12tr.h"
#include "../Tiremo/microphone/tiremo_microphone.h"

#include <string.h>
#include <stdio.h>

#define SENSOR_MIC_CAPTURE_MS   (1000U)

static SensorData_t s_data;

/*-------------------------------------------------------------------*/
static uint32_t Sensor_CaptureMicRms(void)
{
    uint32_t t0;
    uint32_t level;

    if (TIREMO_MIC_App_GetState() == TIREMO_MIC_STATE_RECORDING)
    {
        TIREMO_MIC_App_Stop();
    }

    if (!TIREMO_MIC_App_Start())
    {
        return 0U;
    }

    t0 = TIREMO_SysTick_GetMs();
    while ((TIREMO_SysTick_GetMs() - t0) < SENSOR_MIC_CAPTURE_MS)
    {
        /* DMA capture requires frequent Service() — cannot use blocking DelayMs alone. */
        TIREMO_MIC_BSP_Service();
    }

    TIREMO_MIC_App_Stop();
    level = TIREMO_MIC_App_GetVoiceLevel();
    return level;
}

/*-------------------------------------------------------------------*/
static void Sensor_FillFromDrivers(SensorData_t *pData, uint8_t printErrors)
{
    TiremoBatReading_t bat;
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;

    memset(pData, 0, sizeof(*pData));

    if (TIREMO_SHT40_App_Read(&pData->temperature_mC, &pData->humidity_mRH))
    {
        pData->sht40_ok = 1U;
    }
    else if (printErrors != 0U)
    {
        DebugFramework_PutsLine("[SHT40] Read FAILED");
    }

    if (TIREMO_BAT_App_Read(&bat) && bat.valid)
    {
        pData->battery_ok = 1U;
        pData->battery_mV = bat.avddMv;
    }
    else if (printErrors != 0U)
    {
        DebugFramework_PutsLine("[BATT]  Read FAILED");
    }

    if (TIREMO_LIS2DE12TR_App_ReadAccel(&ax, &ay, &az))
    {
        pData->lis2de12_ok = 1U;
        pData->accel_x_mg  = ax;
        pData->accel_y_mg  = ay;
        pData->accel_z_mg  = az;
    }
    else if (printErrors != 0U)
    {
        DebugFramework_PutsLine("[ACCEL] Read FAILED");
    }

    {
        uint32_t rms = Sensor_CaptureMicRms();
        if (rms > 0U || TIREMO_MIC_App_GetSampleCount() > 0U)
        {
            pData->mic_ok  = 1U;
            pData->mic_rms = rms;
        }
        else if (printErrors != 0U)
        {
            DebugFramework_PutsLine("[MIC]   Capture FAILED");
        }
    }
}

/*-------------------------------------------------------------------*/
void Sensor_LEDTest(void)
{
    DebugFramework_PutsLine("\n\r========================================");
    DebugFramework_PutsLine("         LED HARDWARE TEST");
    DebugFramework_PutsLine("========================================\n\r");

    TIREMO_LED_Init();
    TIREMO_LED_App_Wave(120U);
    TIREMO_LED_AllOff();

    DebugFramework_PutsLine("[OK] LED Test Complete\n\r");
}

/*-------------------------------------------------------------------*/
uint8_t Sensor_TestAll(void)
{
    uint8_t passed = 0U;
    uint8_t failed = 0U;

    DebugFramework_PutsLine("\n\r========================================");
    DebugFramework_PutsLine("       SENSOR INITIALIZATION TEST");
    DebugFramework_PutsLine("========================================\n\r");

    DebugFramework_PutsLine("[TEST] SHT40 Temperature & Humidity...");
    if (TIREMO_SHT40_App_Init())
    {
        DebugFramework_PutsLine("[PASS] SHT40 OK");
        passed++;
    }
    else
    {
        DebugFramework_PutsLine("[FAIL] SHT40 init failed");
        failed++;
    }

    DebugFramework_PutsLine("[TEST] Battery Reading...");
    if (TIREMO_BAT_App_Init())
    {
        DebugFramework_PutsLine("[PASS] Battery OK");
        passed++;
    }
    else
    {
        DebugFramework_PutsLine("[FAIL] Battery init failed");
        failed++;
    }

    DebugFramework_PutsLine("[TEST] LIS2DE12 Accelerometer...");
    if (TIREMO_LIS2DE12TR_App_Init())
    {
        DebugFramework_PutsLine("[PASS] LIS2DE12 OK");
        passed++;
    }
    else
    {
        DebugFramework_PutsLine("[FAIL] LIS2DE12 init failed");
        failed++;
    }

    DebugFramework_PutsLine("[TEST] MP23ABS1 Microphone...");
    if (TIREMO_MIC_App_InitDefault())
    {
        DebugFramework_PutsLine("[PASS] Microphone OK");
        passed++;
    }
    else
    {
        DebugFramework_PutsLine("[FAIL] Microphone init failed");
        failed++;
    }

    DebugFramework_PutsLine("----------------------------------------");
    DebugFramework_Printf("Passed: %u  Failed: %u  Total: %u\n\r",
                          passed, failed, (uint8_t)(passed + failed));
    if (failed == 0U)
    {
        DebugFramework_PutsLine("*** ALL SENSORS OK ***\n\r");
    }
    else
    {
        DebugFramework_Printf("!!! %u SENSOR(S) FAILED !!!\n\r", failed);
    }
    DebugFramework_PutsLine("========================================\n\r");

    return failed;
}

/*-------------------------------------------------------------------*/
SensorData_t *Sensor_ReadOnly(void)
{
    Sensor_FillFromDrivers(&s_data, 0U);
    return &s_data;
}

/*-------------------------------------------------------------------*/
SensorData_t *Sensor_ReadAndPrint(void)
{
    DebugFramework_PutsLine("--- Sensor Readings ---");
    Sensor_FillFromDrivers(&s_data, 1U);

    DebugFramework_Printf("[SHT40] Temp: %ld.%03lu C  Hum: %ld.%03lu %%\n\r",
        (long)(s_data.temperature_mC / 1000),
        (unsigned long)(s_data.temperature_mC >= 0
                            ? (s_data.temperature_mC % 1000)
                            : ((-s_data.temperature_mC) % 1000)),
        (long)(s_data.humidity_mRH / 1000),
        (unsigned long)(s_data.humidity_mRH % 1000));
    DebugFramework_Printf("[BATT]  Voltage: %lu.%03lu V\n\r",
        (unsigned long)(s_data.battery_mV / 1000),
        (unsigned long)(s_data.battery_mV % 1000));
    DebugFramework_Printf("[ACCEL] X:%d  Y:%d  Z:%d mg\n\r",
        s_data.accel_x_mg, s_data.accel_y_mg, s_data.accel_z_mg);
    DebugFramework_Printf("[MIC]   RMS: %lu\n\r", (unsigned long)s_data.mic_rms);

    return &s_data;
}

/*-------------------------------------------------------------------*/
uint16_t Sensor_FormatJSON(const SensorData_t *pData, char *outBuf, uint16_t bufLen)
{
    long temp_int;
    unsigned long temp_frac;
    long hum_int;
    unsigned long hum_frac;
    unsigned long bat_int;
    unsigned long bat_frac;
    int n;

    if ((pData == NULL) || (outBuf == NULL) || (bufLen == 0U))
    {
        return 0U;
    }

    /* Avoid float formatters — they hang on this MCU. */
    temp_int  = (long)(pData->temperature_mC / 1000);
    temp_frac = (unsigned long)(pData->temperature_mC >= 0
                                    ? (pData->temperature_mC % 1000)
                                    : ((-pData->temperature_mC) % 1000));
    hum_int   = (long)(pData->humidity_mRH / 1000);
    hum_frac  = (unsigned long)(pData->humidity_mRH % 1000);
    bat_int   = (unsigned long)(pData->battery_mV / 1000);
    bat_frac  = (unsigned long)(pData->battery_mV % 1000);

    n = snprintf(outBuf, bufLen,
                 "{\"temp\":%ld.%02lu,\"hum\":%ld.%02lu,\"bat\":%lu.%03lu,"
                 "\"ax\":%d,\"ay\":%d,\"az\":%d,\"mic_rms\":%lu}",
                 temp_int, temp_frac / 10U,
                 hum_int, hum_frac / 10U,
                 bat_int, bat_frac,
                 (int)pData->accel_x_mg,
                 (int)pData->accel_y_mg,
                 (int)pData->accel_z_mg,
                 (unsigned long)pData->mic_rms);

    if ((n < 0) || (n >= (int)bufLen))
    {
        n = (int)bufLen - 1;
    }
    return (uint16_t)n;
}

/*-------------------------------------------------------------------*/
/* Alarm detection                                                    */
/*-------------------------------------------------------------------*/

static uint8_t s_tempAbove = 0U;
static uint8_t s_fallActive = 0U;
static uint8_t s_soundAbove = 0U;

uint8_t Sensor_PollAlarms(const SensorData_t *pData,
                          SensorAlarmType_t *pOut,
                          uint8_t maxOut)
{
    uint8_t count = 0U;

    if ((pData == NULL) || (pOut == NULL) || (maxOut == 0U))
    {
        return 0U;
    }

    if (pData->sht40_ok != 0U)
    {
        if (pData->temperature_mC > SENSOR_ALARM_TEMP_THRESHOLD_MC)
        {
            if (s_tempAbove == 0U)
            {
                s_tempAbove = 1U;
                if (count < maxOut)
                {
                    pOut[count++] = SENSOR_ALARM_TEMP_HIGH;
                }
            }
        }
        else if (s_tempAbove != 0U)
        {
            s_tempAbove = 0U;
            if (count < maxOut)
            {
                pOut[count++] = SENSOR_ALARM_TEMP_NORMAL;
            }
        }
    }

    if (pData->lis2de12_ok != 0U)
    {
        if (pData->accel_z_mg <= SENSOR_ALARM_FALL_Z_MAX_MG)
        {
            if (s_fallActive == 0U)
            {
                s_fallActive = 1U;
                if (count < maxOut)
                {
                    pOut[count++] = SENSOR_ALARM_FALL;
                }
            }
        }
        else if (s_fallActive != 0U)
        {
            s_fallActive = 0U;
            if (count < maxOut)
            {
                pOut[count++] = SENSOR_ALARM_FALL_NORMAL;
            }
        }
    }

    if (pData->mic_ok != 0U)
    {
        if (pData->mic_rms > SENSOR_ALARM_MIC_RMS_THRESHOLD)
        {
            if (s_soundAbove == 0U)
            {
                s_soundAbove = 1U;
                if (count < maxOut)
                {
                    pOut[count++] = SENSOR_ALARM_LOUD_SOUND;
                }
            }
        }
        else if (s_soundAbove != 0U)
        {
            s_soundAbove = 0U;
            if (count < maxOut)
            {
                pOut[count++] = SENSOR_ALARM_SOUND_NORMAL;
            }
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

    if ((pData == NULL) || (outBuf == NULL) || (bufLen == 0U))
    {
        return 0U;
    }

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

    if ((n < 0) || (n >= (int)bufLen))
    {
        return 0U;
    }

    return (uint16_t)n;
}
