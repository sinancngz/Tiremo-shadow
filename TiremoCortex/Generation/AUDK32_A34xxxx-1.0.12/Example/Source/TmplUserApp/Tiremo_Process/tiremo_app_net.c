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
#include "../Mqtt_Library/EMPA_MqttAws.h"
#include "../Mqtt_Library/mqtt_core.h"
#include "../Mqtt_Library/mqtt_port.h"
#include "../Mqtt_Library/cert_Lib/mqtt_cert_provision.h"
#include "../Mqtt_Library/cert_Lib/mqtt_certs.h"

extern void MqttPort_ABOV_Init(void);
extern char mqttPacketBuffer[];

uint8_t TiremoAppNet_InitAndConnect(void)
{
    uint8_t connected = 0U;
    uint8_t alreadyProvisioned;

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

#if MQTT_USE_TLS_CERTS
    alreadyProvisioned = MqttFleet_IsProvisioned();

    if (alreadyProvisioned != 0U)
    {
        DebugFramework_PutsLine("[FLEET] Already provisioned — permanent cert on ESP32");
    }
    else
    {
        /*
         * Marker missing: do NOT upload bootstrap yet — that would wipe
         * permanent certs still on ESP32 from a previous successful fleet.
         * Try connecting with whatever certs ESP32 already has.
         */
        DebugFramework_PutsLine("[FLEET] Marker missing — try existing ESP32 certs");
        if (MQTT_ConnectBrokerQuiet() == 0)
        {
            DebugFramework_PutsLine("[FLEET] Existing certs work — skip fleet");
            /* Re-save marker so next boot skips the probe */
            if (MqttFleet_MarkProvisioned(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
            {
                DebugFramework_PutsLine("[FLEET] WARN: could not re-save marker");
            }
            alreadyProvisioned = 1U;
        }
        else
        {
            DebugFramework_PutsLine("[FLEET] No usable certs — bootstrap fleet path");

            if (MqttFleet_UploadBootstrap(mqttPacketBuffer, MQTT_DATA_PACKET_BUFF_SIZE) != 0)
            {
                DebugFramework_PutsLine("[FLEET] Bootstrap upload failed");
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

            DebugFramework_PutsLine("[FLEET] Reconnecting with permanent certificate...");
        }
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
        DebugFramework_PutsLine(MQTT_CLIENT_ID);
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

    if (*pConnected != 0U)
    {
        DebugFramework_PutsLine("[MQTT] Data sending...");
        MQTT_PublishSensorData(pData);
        DebugFramework_Printf("[MQTT] Data sent! #%d\n\r", (*pDataCount)++);
        return;
    }

    TIREMO_LED_Off(TIREMO_LED_6);
    TIREMO_LED_Off(TIREMO_LED_7);

    *pConnected = (MQTT_ConnectBroker() == 0) ? 1U : 0U;
    if (*pConnected != 0U)
    {
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

#endif /* EMPA_ESP32_MQTT_AWS */
