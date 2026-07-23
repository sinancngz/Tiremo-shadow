/**
 *******************************************************************************
 * @file        tiremo_app_net.c
 * @brief       ESP32 WiFi + MQTT + Tiremo fleet provisioning
 ******************************************************************************/

#include "../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS)

#include "tiremo_app_net.h"
#include "../Tiremo/DebugLibrary/debug_framework.h"
#include "../Tiremo/esp32/tiremo_esp32.h"
#include "../Tiremo/led/tiremo_led.h"
#include "../Tiremo/common/tiremo_systick.h"
#include "../Mqtt_Library/EMPA_MqttAws.h"
#include "../Mqtt_Library/mqtt_core.h"
#include "../Mqtt_Library/mqtt_port.h"
#include "../Mqtt_Library/mqtt_device_identity.h"
#include "../Mqtt_Library/cert_Lib/mqtt_cert_provision.h"
#include "../Mqtt_Library/cert_Lib/mqtt_certs.h"

#if (MQTT_SHADOW_ENABLE != 0)
#include "tiremo_shadow.h"
#endif

extern void MqttPort_ABOV_Init(void);
extern char mqttPacketBuffer[];

uint8_t TiremoAppNet_InitAndConnect(void)
{
    uint8_t connected = 0U;
    uint8_t alreadyProvisioned;

#if (MQTT_SHADOW_ENABLE != 0)
    TiremoShadow_Init();
#endif

    DebugFramework_PutsLine("\n\r========================================");
    DebugFramework_PutsLine("   ESP32 / MQTT / FLEET PROVISIONING");
    DebugFramework_PutsLine("========================================\n\r");

    DebugFramework_PutsLine("[TEST] ESP32-AT WiFi Module...");
    if (TIREMO_ESP32_App_Init() && TIREMO_ESP32_App_RunAtTestWithRecovery())
    {
        DebugFramework_PutsLine("[PASS] ESP32 OK");
    }
    else
    {
        DebugFramework_PutsLine("[FAIL] ESP32 NOT responding");
    }

    MqttPort_ABOV_Init();

    /* 1) Client ID = product_macHex12 */
    if (MqttIdentity_InitFromMac(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
    {
        DebugFramework_PutsLine("[ID] Init failed");
        return 0U;
    }
    MQTT_ApplyRuntimeIdentity();

    /* Fleet provisioning — only path to Tiremo (mutual TLS). */
    alreadyProvisioned = MqttFleet_IsProvisioned();

    if (alreadyProvisioned != 0U)
    {
        DebugFramework_PutsLine("[FLEET] Already provisioned for this MAC — keep ESP certs");
    }
    else
    {
        DebugFramework_PutsLine("[FLEET] Fetch+embed certs for clientId:");
        DebugFramework_PutsLine(MqttIdentity_GetClientId());

        if (MqttFleet_UploadBootstrap(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
        {
            DebugFramework_PutsLine("[FLEET] Bootstrap embed to ESP failed");
            return 0U;
        }

        DebugFramework_PutsLine("[FLEET] Connecting with bootstrap certificate...");
        if (MQTT_ConnectBrokerQuiet() != 0)
        {
            DebugFramework_PutsLine("[FLEET] Bootstrap MQTT connect failed");
            return 0U;
        }

        if (MqttFleet_Run(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
        {
            DebugFramework_PutsLine("[FLEET] Fleet exchange failed");
            return 0U;
        }

        DebugFramework_PutsLine("[FLEET] Permanent certs for this clientId embedded in ESP");
    }

    DebugFramework_PutsLine("\n\r[MQTT] Connecting to Tiremo broker (TLS)...");
    connected = (MQTT_ConnectBroker() == 0) ? 1U : 0U;
    if (connected != 0U)
    {
        DebugFramework_PutsLine("[MQTT] Ready to publish sensor data\n\r");
        DebugFramework_PutsLine("[MQTT] Client ID / topics use:");
        DebugFramework_PutsLine(MqttIdentity_GetClientId());

#if (MQTT_SHADOW_ENABLE != 0)
        if (TiremoShadow_OnConnected(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
        {
            DebugFramework_PutsLine("[SHADOW] WARN: shadow setup failed (telemetry continues)");
        }
#endif
    }
    else
    {
        DebugFramework_PutsLine("[MQTT] Will retry each cycle\n\r");
    }

    return connected;
}

/**
 * @brief  WiFi check + quiet MQTT reconnect (no hello flood).
 */
static uint8_t TiremoAppNet_RecoverLink(uint8_t *pConnected)
{
    uint8_t wifiOk;

    if (pConnected == NULL)
    {
        return 0U;
    }

    *pConnected = 0U;
    LED_Mqttconnected(0);

    wifiOk = Wifi_IsStationConnected(mqttPacketBuffer, WIFI_SSID);
    if (wifiOk == 0U)
    {
        DebugFramework_PutsLine("[MQTT] WiFi down — full reconnect...");
        LED_WifiConnected(0);
    }
    else
    {
        DebugFramework_PutsLine("[MQTT] WiFi OK — MQTT reconnect...");
        LED_WifiConnected(1);
        (void)Wifi_MqttClean2(mqttPacketBuffer, POLLING_MODE);
        TIREMO_SysTick_DelayMs(300U);
    }

    *pConnected = (MQTT_ConnectBrokerQuiet() == 0) ? 1U : 0U;
    if (*pConnected != 0U)
    {
        DebugFramework_PutsLine("[MQTT] Reconnected OK");
#if (MQTT_SHADOW_ENABLE != 0)
        if (TiremoShadow_OnConnected(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
        {
            DebugFramework_PutsLine("[SHADOW] WARN: shadow re-setup failed");
        }
#endif
    }
    else
    {
        DebugFramework_PutsLine("[MQTT] Reconnect failed — will retry next cycle");
    }

    return *pConnected;
}

void TiremoAppNet_PublishCycle(const SensorData_t *pData,
                               uint8_t *pConnected,
                               int *pDataCount)
{
    FUNC_StatusTypeDef st = FUNC_FAIL;
    uint8_t attempt;

    if ((pData == NULL) || (pConnected == NULL) || (pDataCount == NULL))
    {
        return;
    }

#if (MQTT_SHADOW_ENABLE != 0)
    if (*pConnected != 0U)
    {
        TiremoShadow_Poll(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE);
    }
#endif

    if (*pConnected == 0U)
    {
#if (MQTT_SHADOW_ENABLE == 0)
        TIREMO_LED_Off(TIREMO_LED_6);
        TIREMO_LED_Off(TIREMO_LED_7);
#endif
        if (TiremoAppNet_RecoverLink(pConnected) == 0U)
        {
            return;
        }
    }

    DebugFramework_PutsLine("[MQTT] Data sending...");
    for (attempt = 1U; attempt <= APP_MQTT_PUBLISH_RETRIES; attempt++)
    {
        st = MQTT_PublishSensorData(pData);
        if (st == FUNC_OK)
        {
            break;
        }
        DebugFramework_Printf("[MQTT] publish failed (st=%u) try %u/%u\n\r",
                              (unsigned)st, (unsigned)attempt,
                              (unsigned)APP_MQTT_PUBLISH_RETRIES);
        if (attempt < APP_MQTT_PUBLISH_RETRIES)
        {
            TIREMO_SysTick_DelayMs(200U);
        }
    }

    if (st != FUNC_OK)
    {
        DebugFramework_PutsLine("[MQTT] retries exhausted — recover link");
        if (TiremoAppNet_RecoverLink(pConnected) == 0U)
        {
            return;
        }
        DebugFramework_PutsLine("[MQTT] Data sending (after recover)...");
        st = MQTT_PublishSensorData(pData);
        if (st != FUNC_OK)
        {
            DebugFramework_Printf("[MQTT] still failed (st=%u)\n\r", (unsigned)st);
            *pConnected = 0U;
            LED_Mqttconnected(0);
            return;
        }
    }

    DebugFramework_Printf("[MQTT] Data sent! #%d\n\r", (*pDataCount)++);

#if (MQTT_SHADOW_ENABLE != 0)
    TiremoShadow_Poll(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE);
#endif
}

void TiremoAppNet_PublishAlarms(const SensorData_t *pData,
                                const SensorAlarmType_t *alarms,
                                uint8_t alarmCount,
                                uint8_t connected)
{
    char alarmBuf[192];
    uint8_t ai;
    FUNC_StatusTypeDef st;
    static const char *const names[] = {
        "TEMPERATURE_HIGH",
        "TEMPERATURE_NORMAL",
        "FALL_DETECTED",
        "FALL_NORMAL",
        "LOUD_SOUND",
        "SOUND_NORMAL"
    };

    if ((pData == NULL) || (alarms == NULL) || (alarmCount == 0U))
    {
        return;
    }

    for (ai = 0U; ai < alarmCount; ai++)
    {
        const char *name = "UNKNOWN";
        if ((unsigned)alarms[ai] < (sizeof(names) / sizeof(names[0])))
        {
            name = names[alarms[ai]];
        }

        if (Sensor_FormatAlarmJSON(alarms[ai], pData, alarmBuf, sizeof(alarmBuf)) == 0U)
        {
            DebugFramework_Printf("[ALARM] %s — format failed\n\r", name);
            continue;
        }

        DebugFramework_Printf("[ALARM] %s\n\r", name);
        DebugFramework_PutsLine(alarmBuf);

        if (connected == 0U)
        {
            DebugFramework_PutsLine("[ALARM] MQTT not connected — not published");
            continue;
        }

        st = MQTT_PublishAlarm(alarmBuf);
        if (st == FUNC_OK)
        {
            DebugFramework_PutsLine("[ALARM] published OK");
        }
        else
        {
            DebugFramework_Printf("[ALARM] publish failed (st=%u)\n\r", (unsigned)st);
        }
    }
}

uint32_t TiremoAppNet_GetPublishIntervalMs(void)
{
#if (MQTT_SHADOW_ENABLE != 0)
    return TiremoShadow_GetPublishIntervalMs();
#else
    return APP_PUBLISH_INTERVAL_MS;
#endif
}

void TiremoAppNet_IdleService(uint8_t connected, uint32_t durationMs)
{
#if (MQTT_SHADOW_ENABLE != 0)
    if (connected != 0U)
    {
        TiremoShadow_ServiceForMs(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE,
                                  durationMs);
        return;
    }
#endif
    (void)connected;
    TIREMO_SysTick_DelayMs(durationMs);
}

#endif /* EMPA_ESP32_MQTT_AWS */
