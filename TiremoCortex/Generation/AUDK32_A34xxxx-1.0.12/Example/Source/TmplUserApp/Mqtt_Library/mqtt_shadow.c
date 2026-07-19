/**
 * @file    mqtt_shadow.c
 * @brief   AWS IoT classic Device Shadow over ESP-AT MQTT
 */

#include "../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)

#include "mqtt_shadow.h"
#include "mqtt_core.h"
#include "mqtt_port.h"
#include "../Tiremo/DebugLibrary/debug_framework.h"

#include <stdio.h>
#include <string.h>

#define SHADOW_RAW_MAX       3072U
#define SHADOW_PAYLOAD_MAX   2048U

static char s_raw[SHADOW_RAW_MAX];
static char s_payload[SHADOW_PAYLOAD_MAX];

/* ---- JSON helpers ------------------------------------------------------ */

int MqttShadow_ExtractString(const char *json, const char *key,
                             char *out, uint16_t outMax)
{
    char pattern[64];
    const char *p;
    uint16_t oi = 0U;

    if ((json == NULL) || (key == NULL) || (out == NULL) || (outMax < 2U))
    {
        return -1;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
    p = strstr(json, pattern);
    if (p == NULL)
    {
        return -1;
    }
    p += strlen(pattern);

    while ((*p != '\0') && (oi < (outMax - 1U)))
    {
        if ((*p == '\\') && (p[1] != '\0'))
        {
            if (p[1] == 'n')
            {
                out[oi++] = '\n';
                p += 2;
                continue;
            }
            if ((p[1] == '"') || (p[1] == '\\') || (p[1] == '/'))
            {
                out[oi++] = p[1];
                p += 2;
                continue;
            }
        }
        if (*p == '"')
        {
            break;
        }
        out[oi++] = *p++;
    }
    out[oi] = '\0';
    return (oi > 0U) ? 0 : -1;
}

int MqttShadow_ExtractInt(const char *json, const char *key, int32_t *out)
{
    char pattern[64];
    const char *p;
    int32_t sign = 1;
    int32_t val = 0;
    uint8_t digits = 0U;

    if ((json == NULL) || (key == NULL) || (out == NULL))
    {
        return -1;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    p = strstr(json, pattern);
    if (p == NULL)
    {
        return -1;
    }
    p += strlen(pattern);
    while ((*p == ' ') || (*p == '\t'))
    {
        p++;
    }
    if (*p == '-')
    {
        sign = -1;
        p++;
    }
    while ((*p >= '0') && (*p <= '9'))
    {
        val = (val * 10) + (int32_t)(*p - '0');
        digits = 1U;
        p++;
    }
    if (digits == 0U)
    {
        return -1;
    }
    *out = val * sign;
    return 0;
}

int MqttShadow_ExtractBool(const char *json, const char *key, uint8_t *out)
{
    char pattern[64];
    const char *p;

    if ((json == NULL) || (key == NULL) || (out == NULL))
    {
        return -1;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    p = strstr(json, pattern);
    if (p == NULL)
    {
        return -1;
    }
    p += strlen(pattern);
    while ((*p == ' ') || (*p == '\t'))
    {
        p++;
    }

    if (strncmp(p, "true", 4) == 0)
    {
        *out = 1U;
        return 0;
    }
    if (strncmp(p, "false", 5) == 0)
    {
        *out = 0U;
        return 0;
    }
    if (*p == '1')
    {
        *out = 1U;
        return 0;
    }
    if (*p == '0')
    {
        *out = 0U;
        return 0;
    }
    return -1;
}

const char *MqttShadow_FindObject(const char *json, const char *key)
{
    char pattern[64];
    const char *p;

    if ((json == NULL) || (key == NULL))
    {
        return NULL;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\":{", key);
    p = strstr(json, pattern);
    if (p == NULL)
    {
        /* allow whitespace: "key" : { */
        snprintf(pattern, sizeof(pattern), "\"%s\"", key);
        p = strstr(json, pattern);
        if (p == NULL)
        {
            return NULL;
        }
        p += strlen(pattern);
        while ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n') || (*p == ':'))
        {
            p++;
        }
        if (*p != '{')
        {
            return NULL;
        }
        return p;
    }
    return p + strlen(pattern) - 1U; /* point at '{' */
}

/* ---- UART SUBRECV collect ---------------------------------------------- */

static uint16_t shadow_extract_payloads(const char *raw, uint16_t rawLen,
                                        char *out, uint16_t outMax)
{
    uint16_t i = 0U;
    uint16_t assembled = 0U;

    if ((raw == NULL) || (out == NULL) || (outMax < 2U))
    {
        return 0U;
    }
    out[0] = '\0';

    while ((i + 13U) < rawLen)
    {
        const char *hit = strstr(&raw[i], "+MQTTSUBRECV:");
        const char *p;
        unsigned len = 0U;
        uint16_t pi;
        unsigned k;

        if (hit == NULL)
        {
            break;
        }
        i = (uint16_t)(hit - raw);
        p = &raw[i] + 13;

        while ((p < (raw + rawLen)) && (*p != ','))
        {
            p++;
        }
        if ((p >= (raw + rawLen)) || (*p != ','))
        {
            break;
        }
        p++;
        if (*p != '"')
        {
            i = (uint16_t)(p - raw);
            continue;
        }
        p++;
        while ((p < (raw + rawLen)) && (*p != '"'))
        {
            p++;
        }
        if ((p >= (raw + rawLen)) || (*p != '"'))
        {
            break;
        }
        p++;
        if (*p != ',')
        {
            i = (uint16_t)(p - raw);
            continue;
        }
        p++;

        len = 0U;
        while ((p < (raw + rawLen)) && (*p >= '0') && (*p <= '9'))
        {
            len = (len * 10U) + (unsigned)(*p - '0');
            p++;
        }
        if ((p >= (raw + rawLen)) || (*p != ',') || (len == 0U))
        {
            i = (uint16_t)(p - raw);
            continue;
        }
        p++;

        pi = (uint16_t)(p - raw);
        if (((uint32_t)pi + (uint32_t)len) > (uint32_t)rawLen)
        {
            break;
        }

        if (assembled > 0U)
        {
            /* Separate multiple frames with a sentinel newline for handlers. */
            if ((assembled + 1U) < outMax)
            {
                out[assembled++] = '\n';
                out[assembled] = '\0';
            }
        }

        for (k = 0U; k < len; k++)
        {
            if ((assembled + 1U) < outMax)
            {
                out[assembled++] = raw[pi + (uint16_t)k];
                out[assembled] = '\0';
            }
        }
        i = (uint16_t)(pi + len);
    }

    return assembled;
}

/* ---- Public API -------------------------------------------------------- */

int MqttShadow_Subscribe(char *buffer, uint16_t bufSize)
{
    (void)bufSize;

    if (buffer == NULL)
    {
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Subscribe delta...");
    if (Wifi_MqttSub2(buffer, MQTT_SHADOW_TOPIC_DELTA, 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] Sub delta FAILED");
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Subscribe get/accepted...");
    if (Wifi_MqttSub2(buffer, MQTT_SHADOW_TOPIC_GET_ACCEPTED, 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] Sub get/accepted FAILED");
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Subscribe update/accepted...");
    if (Wifi_MqttSub2(buffer, MQTT_SHADOW_TOPIC_UPD_ACCEPTED, 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] WARN: Sub update/accepted failed");
    }

    if (Wifi_MqttSub2(buffer, MQTT_SHADOW_TOPIC_GET_REJECTED, 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] WARN: Sub get/rejected failed");
    }

    if (Wifi_MqttSub2(buffer, MQTT_SHADOW_TOPIC_UPD_REJECTED, 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] WARN: Sub update/rejected failed");
    }

    DebugFramework_PutsLine("[SHADOW] Subscriptions OK");
    return 0;
}

int MqttShadow_RequestGet(char *buffer, uint16_t bufSize)
{
    (void)bufSize;

    if (buffer == NULL)
    {
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Request GET...");
    if (Wifi_MqttPubRaw2(buffer, (char *)MQTT_SHADOW_TOPIC_GET,
                         2U, "{}", QOS_1, RTN_0, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] GET publish failed");
        return -1;
    }
    return 0;
}

int MqttShadow_PublishReported(char *buffer, uint16_t bufSize,
                               const char *reportedBody)
{
    char envelope[768];
    int n;

    if ((buffer == NULL) || (reportedBody == NULL) || (bufSize < 128U))
    {
        return -1;
    }

    n = snprintf(envelope, sizeof(envelope),
                 "{\"state\":{\"reported\":%s}}", reportedBody);
    if ((n <= 0) || (n >= (int)sizeof(envelope)))
    {
        DebugFramework_PutsLine("[SHADOW] reported JSON too large");
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Publish reported...");
    if (Wifi_MqttPubRaw2(buffer, (char *)MQTT_SHADOW_TOPIC_UPDATE,
                         (uint16_t)strlen(envelope), envelope,
                         QOS_1, RTN_0, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] reported publish failed");
        return -1;
    }
    return 0;
}

/* ---- UART SUBRECV collect ---------------------------------------------- */

/**
 * Continuous UART collect for windowMs.
 * Once +MQTTSUBRECV is seen, keep reading until silence
 * (MQTT_SHADOW_SUBRECV_SILENCE_MS) or hard deadline (windowMs + extra).
 * Do NOT use a short silence — that truncates large shadow JSON.
 */
static uint16_t shadow_slurp(char *raw, uint16_t rawMax, uint32_t windowMs)
{
    const MqttPort_Interface *port = MqttPort_Get();
    uint32_t t0;
    uint32_t lastRx;
    uint32_t deadline;
    uint16_t n = 0U;
    uint8_t sawSub = 0U;
    uint8_t matchIdx = 0U;
    static const char s_tag[] = "+MQTTSUBRECV:";
    const uint32_t silenceMs = MQTT_SHADOW_SUBRECV_SILENCE_MS;

    if ((port == NULL) || (raw == NULL) || (rawMax < 16U))
    {
        return 0U;
    }

    memset(raw, 0, rawMax);
    t0 = port->get_tick_ms();
    lastRx = t0;
    deadline = windowMs;

    while ((port->get_tick_ms() - t0) < deadline)
    {
        uint8_t b;
        if (port->uart_receive(&b, 1U, 5U) == MQTT_PORT_OK)
        {
            if ((n + 1U) < rawMax)
            {
                raw[n++] = (char)b;
                raw[n] = '\0';
            }
            lastRx = port->get_tick_ms();

            if (sawSub == 0U)
            {
                if ((char)b == s_tag[matchIdx])
                {
                    matchIdx++;
                    if (s_tag[matchIdx] == '\0')
                    {
                        sawSub = 1U;
                        /* Allow enough time to finish a large SUBRECV body. */
                        {
                            uint32_t elapsed = port->get_tick_ms() - t0;
                            uint32_t need = elapsed + 4000U;
                            if (need > deadline)
                            {
                                deadline = need;
                            }
                        }
                    }
                }
                else if ((char)b == s_tag[0])
                {
                    matchIdx = 1U;
                }
                else
                {
                    matchIdx = 0U;
                }
            }
            continue;
        }

        if ((sawSub != 0U) &&
            ((port->get_tick_ms() - lastRx) >= silenceMs))
        {
            break;
        }
    }
    return n;
}

static int shadow_dispatch_payload(int (*onDeltaJson)(const char *json, void *user),
                                   void *user)
{
    const char *deltaNested;
    const char *stateObj;
    const char *desiredObj;
    int rc;

    deltaNested = MqttShadow_FindObject(s_payload, "delta");
    if (deltaNested != NULL)
    {
        DebugFramework_PutsLine("[SHADOW] apply: delta object");
        rc = onDeltaJson(deltaNested, user);
        return (rc == 0) ? 1 : -1;
    }

    stateObj = MqttShadow_FindObject(s_payload, "state");
    if (stateObj == NULL)
    {
        DebugFramework_PutsLine("[SHADOW] no state object — ignore");
        return 0;
    }

    if ((MqttShadow_FindObject(stateObj, "desired") == NULL) &&
        (MqttShadow_FindObject(stateObj, "reported") == NULL) &&
        (MqttShadow_FindObject(stateObj, "delta") == NULL))
    {
        DebugFramework_PutsLine("[SHADOW] apply: update/delta state");
        rc = onDeltaJson(stateObj, user);
        return (rc == 0) ? 1 : -1;
    }

    desiredObj = MqttShadow_FindObject(stateObj, "desired");
    if (desiredObj != NULL)
    {
        DebugFramework_PutsLine("[SHADOW] apply: desired fallback");
        rc = onDeltaJson(desiredObj, user);
        return (rc == 0) ? 1 : -1;
    }

    DebugFramework_PutsLine("[SHADOW] no delta/desired — ignore");
    return 0;
}

static int shadow_poll_internal(uint32_t windowMs,
                                int (*onDeltaJson)(const char *json, void *user),
                                void *user)
{
    uint16_t rawLen;
    uint16_t payloadLen;

    if (onDeltaJson == NULL)
    {
        return -1;
    }

    rawLen = shadow_slurp(s_raw, (uint16_t)sizeof(s_raw), windowMs);
    if (rawLen == 0U)
    {
        return 0;
    }

    if (strstr(s_raw, "+MQTTSUBRECV:") == NULL)
    {
        return 0;
    }

    payloadLen = shadow_extract_payloads(s_raw, rawLen, s_payload,
                                         (uint16_t)sizeof(s_payload));
    if (payloadLen == 0U)
    {
        DebugFramework_Printf("[SHADOW] SUBRECV seen but parse fail raw=%u\n\r",
                              (unsigned)rawLen);
        return 0;
    }

    DebugFramework_Printf("[SHADOW] RX payload=%u raw=%u\n\r",
                          (unsigned)payloadLen, (unsigned)rawLen);
    {
        char preview[97];
        uint16_t i;
        uint16_t n = (payloadLen > 96U) ? 96U : payloadLen;
        for (i = 0U; i < n; i++)
        {
            char c = s_payload[i];
            preview[i] = ((c >= 32) && (c < 127)) ? c : '.';
        }
        preview[n] = '\0';
        DebugFramework_PutsLine(preview);
    }

    return shadow_dispatch_payload(onDeltaJson, user);
}

int MqttShadow_Poll(char *buffer, uint16_t bufSize,
                    int (*onDeltaJson)(const char *json, void *user),
                    void *user)
{
    (void)buffer;
    (void)bufSize;
    return shadow_poll_internal(MQTT_SHADOW_POLL_MS, onDeltaJson, user);
}

int MqttShadow_PollWindow(char *buffer, uint16_t bufSize, uint32_t windowMs,
                          int (*onDeltaJson)(const char *json, void *user),
                          void *user)
{
    (void)buffer;
    (void)bufSize;
    if (windowMs < 100U)
    {
        windowMs = 100U;
    }
    return shadow_poll_internal(windowMs, onDeltaJson, user);
}

#endif /* EMPA_ESP32_MQTT_AWS && MQTT_SHADOW_ENABLE */
