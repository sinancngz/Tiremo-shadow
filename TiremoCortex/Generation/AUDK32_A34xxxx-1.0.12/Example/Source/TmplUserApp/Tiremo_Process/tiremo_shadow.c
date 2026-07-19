/**
 * @file    tiremo_shadow.c
 * @brief   Apply AWS Shadow delta: lightState → all LEDs, then report
 *
 * RX is polling (not IRQ). Must keep draining ESP32 UART between publishes
 * or +MQTTSUBRECV frames are lost during DelayMs.
 */

#include "../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)

#include "tiremo_shadow.h"
#include "../Mqtt_Library/mqtt_shadow.h"
#include "../Tiremo/DebugLibrary/debug_framework.h"
#include "../Tiremo/led/tiremo_led.h"
#include "../Tiremo/common/tiremo_systick.h"

#include <stdio.h>
#include <string.h>

static TiremoShadowState_t s_state;
static uint8_t s_dirty; /* report after successful delta apply */

static void shadow_apply_light(void)
{
    if (s_state.lightState != 0U)
    {
        TIREMO_LED_AllOn();
        DebugFramework_PutsLine("[SHADOW] lightState=true → ALL LEDs ON");
    }
    else
    {
        TIREMO_LED_AllOff();
        DebugFramework_PutsLine("[SHADOW] lightState=false → ALL LEDs OFF");
    }
}

static int shadow_publish_reported(char *buffer, uint16_t bufSize)
{
    char body[48];

    snprintf(body, sizeof(body),
             "{\"lightState\":%s}",
             (s_state.lightState != 0U) ? "true" : "false");

    if (MqttShadow_PublishReported(buffer, bufSize, body) != 0)
    {
        return -1;
    }
    s_dirty = 0U;
    return 0;
}

static int shadow_apply_delta(const char *deltaJson)
{
    uint8_t light = 0U;

    if (deltaJson == NULL)
    {
        return 0;
    }

    DebugFramework_PutsLine("[SHADOW] Applying lightState...");

    if (MqttShadow_ExtractBool(deltaJson, "lightState", &light) != 0)
    {
        /* Some payloads nest again or use quoted bool — try full-buffer search. */
        DebugFramework_PutsLine("[SHADOW] lightState not in object — scan ignore");
        return 0;
    }

    /* Always drive LEDs (even if value unchanged — recovers after LED test). */
    s_state.lightState = light;
    shadow_apply_light();
    s_dirty = 1U;
    return 1;
}

static int shadow_on_delta(const char *deltaJson, void *user)
{
    (void)user;
    (void)shadow_apply_delta(deltaJson);
    return 0;
}

void TiremoShadow_Init(void)
{
    memset(&s_state, 0, sizeof(s_state));
    s_state.lightState = 0U;
    s_state.subscribed = 0U;
    s_dirty = 0U;
    TIREMO_LED_AllOff();
    DebugFramework_PutsLine("[SHADOW] Init OK (lightState → all LEDs)");
}

int TiremoShadow_OnConnected(char *buffer, uint16_t bufSize)
{
    if ((buffer == NULL) || (bufSize < 256U))
    {
        return -1;
    }

    if (MqttShadow_Subscribe(buffer, bufSize) != 0)
    {
        s_state.subscribed = 0U;
        return -1;
    }
    s_state.subscribed = 1U;

    /* Fetch pending delta — one continuous listen (do not split SUBRECV). */
    (void)MqttShadow_RequestGet(buffer, bufSize);
    DebugFramework_PutsLine("[SHADOW] Waiting for get/delta...");
    (void)MqttShadow_PollWindow(buffer, bufSize, MQTT_SHADOW_GET_WAIT_MS,
                                shadow_on_delta, NULL);

    if (s_dirty != 0U)
    {
        if (shadow_publish_reported(buffer, bufSize) != 0)
        {
            DebugFramework_PutsLine("[SHADOW] WARN: reported after delta failed");
            return -1;
        }
    }
    else
    {
        DebugFramework_PutsLine("[SHADOW] no pending delta");
    }

    shadow_apply_light();
    DebugFramework_PutsLine("[SHADOW] OnConnected complete (continuous UART listen)");
    return 0;
}

void TiremoShadow_Poll(char *buffer, uint16_t bufSize)
{
    if ((s_state.subscribed == 0U) || (buffer == NULL))
    {
        return;
    }

    (void)MqttShadow_Poll(buffer, bufSize, shadow_on_delta, NULL);

    if (s_dirty != 0U)
    {
        (void)shadow_publish_reported(buffer, bufSize);
    }
}

void TiremoShadow_ServiceForMs(char *buffer, uint16_t bufSize, uint32_t durationMs)
{
    if ((buffer == NULL) || (s_state.subscribed == 0U))
    {
        TIREMO_SysTick_DelayMs(durationMs);
        return;
    }

    /*
     * ONE continuous listen — do not slice into short polls.
     * Short polls were splitting +MQTTSUBRECV across windows → parse fail → "no RX".
     */
    if (durationMs < MQTT_SHADOW_POLL_MS)
    {
        durationMs = MQTT_SHADOW_POLL_MS;
    }

    (void)MqttShadow_PollWindow(buffer, bufSize, durationMs, shadow_on_delta, NULL);

    if (s_dirty != 0U)
    {
        (void)shadow_publish_reported(buffer, bufSize);
    }
}

uint32_t TiremoShadow_GetPublishIntervalMs(void)
{
    return APP_PUBLISH_INTERVAL_MS;
}

const char *TiremoShadow_GetPubTopic(void)
{
    return MQTT_TOPIC_PUB;
}

const char *TiremoShadow_GetAlarmTopic(void)
{
    return MQTT_TOPIC_ALARM;
}

const TiremoShadowState_t *TiremoShadow_GetState(void)
{
    return &s_state;
}

#endif /* EMPA_ESP32_MQTT_AWS && MQTT_SHADOW_ENABLE */
