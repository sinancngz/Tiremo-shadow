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

#if MQTT_USE_TLS_CERTS
    /* 2) Skip fleet only if ESP marker matches THIS MAC deviceId */
    alreadyProvisioned = MqttFleet_IsProvisioned();

    if (alreadyProvisioned != 0U)
    {
        DebugFramework_PutsLine("[FLEET] Already provisioned for this MAC — keep ESP certs");
    }
    else
    {
        /*
         * Do NOT trust existing ESP certs: they may belong to another Thing.
         * Always fetch certs for the MAC-derived clientId and embed into ESP.
         */
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
#else
    (void)alreadyProvisioned;
#endif

    DebugFramework_PutsLine("\n\r[MQTT] Connecting to broker...");
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

void TiremoAppNet_PublishCycle(const SensorData_t *pData,
                               uint8_t *pConnected,
                               int *pDataCount)
{
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

    if (*pConnected != 0U)
    {
        DebugFramework_PutsLine("[MQTT] Data sending...");
        MQTT_PublishSensorData(pData);
        DebugFramework_Printf("[MQTT] Data sent! #%d\n\r", (*pDataCount)++);

#if (MQTT_SHADOW_ENABLE != 0)
        /* Catch delta that arrived during / right after publish AT exchange. */
        TiremoShadow_Poll(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE);
#endif
        return;
    }

#if (MQTT_SHADOW_ENABLE == 0)
    TIREMO_LED_Off(TIREMO_LED_6);
    TIREMO_LED_Off(TIREMO_LED_7);
#endif

    *pConnected = (MQTT_ConnectBroker() == 0) ? 1U : 0U;
    if (*pConnected != 0U)
    {
#if (MQTT_SHADOW_ENABLE != 0)
        if (TiremoShadow_OnConnected(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
        {
            DebugFramework_PutsLine("[SHADOW] WARN: shadow re-setup failed");
        }
#endif
        MQTT_PublishSensorData(pData);
    }
}

void TiremoAppNet_PublishAlarms(const SensorData_t *pData,
                                const SensorAlarmType_t *alarms,
                                uint8_t alarmCount,
                                uint8_t connected)
{
    char alarmBuf[160];
    uint8_t ai;

    if ((pData == NULL) || (alarms == NULL) || (alarmCount == 0U) || (connected == 0U))
    {
        return;
    }

    for (ai = 0U; ai < alarmCount; ai++)
    {
        if (Sensor_FormatAlarmJSON(alarms[ai], pData, alarmBuf, sizeof(alarmBuf)) == 0U)
        {
            continue;
        }
        MQTT_PublishAlarm(alarmBuf);
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
