/**
 * @file    tiremo_shadow.c
 * @brief   Device Shadow — lightState on/off (shadow.txt / AWS rules)
 *
 * Log analysis: device DID apply one update/delta (lightState:false) and
 * published reported. If desired stays true, AWS creates a NEW delta(true)
 * that must be caught AFTER reported publish — previously that push was lost.
 *
 * Also: GET responses often arrive during PubRaw AT exchange; we retry GET
 * and always listen immediately after get/reported.
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
static uint8_t s_dirty;
static uint8_t s_gotDoc; /* 1 if any shadow JSON handled in last listen */

static void shadow_apply_light(void)
{
    if (s_state.lightState != 0U)
    {
        TIREMO_LED_AllOn();
        DebugFramework_PutsLine("[SHADOW] LEDs ON (lightState=true)");
    }
    else
    {
        TIREMO_LED_AllOff();
        DebugFramework_PutsLine("[SHADOW] LEDs OFF (lightState=false)");
    }
}

static int shadow_publish_reported(char *buffer, uint16_t bufSize)
{
    char body[40];

    snprintf(body, sizeof(body),
             "{\"lightState\":%s}",
             (s_state.lightState != 0U) ? "true" : "false");

    if (MqttShadow_PublishReported(buffer, bufSize, body) != 0)
    {
        return -1;
    }
    s_dirty = 0U;
    DebugFramework_PutsLine("[SHADOW] reported published");
    return 0;
}

static int shadow_on_document(const char *json, void *user)
{
    const char *deltaMark;
    const char *stateObj;
    const char *reportedMark;
    uint8_t light = 0U;

    (void)user;
    if (json == NULL)
    {
        return -1;
    }

    s_gotDoc = 1U;

    /* 1) Full document with AWS delta section */
    deltaMark = strstr(json, "\"delta\"");
    if (deltaMark != NULL)
    {
        if (MqttShadow_ExtractBool(deltaMark, "lightState", &light) == 0)
        {
            DebugFramework_Printf("[SHADOW] DELTA apply lightState=%u\n\r",
                                  (unsigned)light);
            s_state.lightState = light;
            shadow_apply_light();
            s_dirty = 1U;
            return 0;
        }
    }

    /* 2) Push on .../shadow/update/delta : state holds only changed fields */
    stateObj = MqttShadow_FindObject(json, "state");
    if (stateObj != NULL)
    {
        if ((strstr(stateObj, "\"desired\"") == NULL) &&
            (strstr(stateObj, "\"reported\"") == NULL) &&
            (strstr(stateObj, "\"delta\"") == NULL))
        {
            if (MqttShadow_ExtractBool(stateObj, "lightState", &light) == 0)
            {
                DebugFramework_Printf("[SHADOW] update/delta apply lightState=%u\n\r",
                                      (unsigned)light);
                s_state.lightState = light;
                shadow_apply_light();
                s_dirty = 1U;
                return 0;
            }
        }
    }

    /* 3) No delta — restore last confirmed reported (reboot) */
    reportedMark = strstr(json, "\"reported\"");
    if (reportedMark != NULL)
    {
        if (MqttShadow_ExtractBool(reportedMark, "lightState", &light) == 0)
        {
            DebugFramework_Printf("[SHADOW] restore reported lightState=%u\n\r",
                                  (unsigned)light);
            s_state.lightState = light;
            shadow_apply_light();
            return 0;
        }
    }

    DebugFramework_PutsLine("[SHADOW] doc ignored");
    return 0;
}

/**
 * Listen, apply, publish reported, then listen AGAIN —
 * publishing reported can create a new AWS delta if desired still differs.
 */
static void shadow_sync_round(char *buffer, uint16_t bufSize, uint32_t listenMs)
{
    uint8_t round;

    for (round = 0U; round < 2U; round++)
    {
        s_gotDoc = 0U;
        (void)MqttShadow_PollWindow(buffer, bufSize, listenMs, shadow_on_document, NULL);

        if (s_dirty != 0U)
        {
            (void)shadow_publish_reported(buffer, bufSize);
            /* Catch immediate follow-up delta from AWS */
            listenMs = 3000U;
            continue;
        }
        break;
    }
}

static void shadow_get_and_sync(char *buffer, uint16_t bufSize, uint32_t listenMs)
{
    (void)MqttShadow_RequestGet(buffer, bufSize);
    /* No DelayMs here — UART is polled only; idle delay drops get/accepted. */
    shadow_sync_round(buffer, bufSize, listenMs);
}

void TiremoShadow_Init(void)
{
    memset(&s_state, 0, sizeof(s_state));
    s_state.lightState = 0U;
    s_state.subscribed = 0U;
    s_dirty = 0U;
    s_gotDoc = 0U;
    TIREMO_LED_AllOff();
    DebugFramework_PutsLine("[SHADOW] Init OK");
}

int TiremoShadow_OnConnected(char *buffer, uint16_t bufSize)
{
    uint8_t attempt;

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

    /* Retry GET — first response is often lost in AT TX/RX */
    for (attempt = 0U; attempt < 3U; attempt++)
    {
        DebugFramework_Printf("[SHADOW] sync attempt %u/3\n\r",
                              (unsigned)(attempt + 1U));
        s_gotDoc = 0U;
        shadow_get_and_sync(buffer, bufSize, MQTT_SHADOW_GET_WAIT_MS);
        if (s_gotDoc != 0U)
        {
            break;
        }
        DebugFramework_PutsLine("[SHADOW] WARN: no shadow RX — retry GET");
    }

    DebugFramework_PutsLine("[SHADOW] OnConnected done");
    return 0;
}

void TiremoShadow_Poll(char *buffer, uint16_t bufSize)
{
    if ((s_state.subscribed == 0U) || (buffer == NULL))
    {
        return;
    }
    /* Opportunistic listen only (no GET) — catch push delta */
    shadow_sync_round(buffer, bufSize, MQTT_SHADOW_POLL_MS);
}

void TiremoShadow_ServiceForMs(char *buffer, uint16_t bufSize, uint32_t durationMs)
{
    uint32_t listenMs;

    if ((buffer == NULL) || (s_state.subscribed == 0U))
    {
        TIREMO_SysTick_DelayMs(durationMs);
        return;
    }

    listenMs = durationMs;
    if (listenMs < 3000U)
    {
        listenMs = 3000U;
    }

    shadow_get_and_sync(buffer, bufSize, listenMs);
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
