/**
 * @file    mqtt_shadow.c
 * @brief   AWS IoT classic Device Shadow over ESP-AT (shadow.txt / AWS rules)
 *
 * Device:
 *   1) Subscribe update/delta + get/accepted
 *   2) GET shadow on connect / idle
 *   3) Deliver each MQTT payload to app (app applies delta / restores reported)
 *   4) App publishes reported after applying delta
 */

#include "../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)

#include "mqtt_shadow.h"
#include "mqtt_core.h"
#include "mqtt_port.h"
#include "mqtt_device_identity.h"
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
    char keyPat[48];
    const char *p;
    uint16_t oi = 0U;

    if ((json == NULL) || (key == NULL) || (out == NULL) || (outMax < 2U))
    {
        return -1;
    }

    snprintf(keyPat, sizeof(keyPat), "\"%s\"", key);
    p = strstr(json, keyPat);
    if (p == NULL)
    {
        return -1;
    }
    p += strlen(keyPat);
    while ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))
    {
        p++;
    }
    if (*p != ':')
    {
        return -1;
    }
    p++;
    while ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))
    {
        p++;
    }
    if (*p != '"')
    {
        return -1;
    }
    p++;

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
    char keyPat[48];
    const char *p;
    int32_t sign = 1;
    int32_t val = 0;
    uint8_t digits = 0U;

    if ((json == NULL) || (key == NULL) || (out == NULL))
    {
        return -1;
    }

    snprintf(keyPat, sizeof(keyPat), "\"%s\"", key);
    p = strstr(json, keyPat);
    if (p == NULL)
    {
        return -1;
    }
    p += strlen(keyPat);
    while ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))
    {
        p++;
    }
    if (*p != ':')
    {
        return -1;
    }
    p++;
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
    char keyPat[48];
    const char *p;

    if ((json == NULL) || (key == NULL) || (out == NULL))
    {
        return -1;
    }

    snprintf(keyPat, sizeof(keyPat), "\"%s\"", key);
    p = strstr(json, keyPat);
    if (p == NULL)
    {
        return -1;
    }
    p += strlen(keyPat);
    while ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))
    {
        p++;
    }
    if (*p != ':')
    {
        return -1;
    }
    p++;
    while ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))
    {
        p++;
    }
    if (*p == '"')
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
    char keyPat[48];
    const char *p;

    if ((json == NULL) || (key == NULL))
    {
        return NULL;
    }

    snprintf(keyPat, sizeof(keyPat), "\"%s\"", key);
    p = strstr(json, keyPat);
    if (p == NULL)
    {
        return NULL;
    }
    p += strlen(keyPat);
    while ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))
    {
        p++;
    }
    if (*p != ':')
    {
        return NULL;
    }
    p++;
    while ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))
    {
        p++;
    }
    if (*p != '{')
    {
        return NULL;
    }
    return p;
}

/* ---- UART -------------------------------------------------------------- */

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

static int shadow_poll_internal(uint32_t windowMs,
                                int (*onJson)(const char *json, void *user),
                                void *user)
{
    uint16_t rawLen;
    uint16_t i;
    int handled = 0;

    if (onJson == NULL)
    {
        return -1;
    }

    rawLen = shadow_slurp(s_raw, (uint16_t)sizeof(s_raw), windowMs);
    if ((rawLen == 0U) || (strstr(s_raw, "+MQTTSUBRECV:") == NULL))
    {
        return 0;
    }

    /* One MQTT message per SUBRECV — handle separately. */
    i = 0U;
    while ((i + 13U) < rawLen)
    {
        const char *hit = strstr(&s_raw[i], "+MQTTSUBRECV:");
        const char *p;
        unsigned len = 0U;
        uint16_t pi;
        unsigned k;
        uint16_t payloadLen;

        if (hit == NULL)
        {
            break;
        }
        i = (uint16_t)(hit - s_raw);
        p = &s_raw[i] + 13;

        while ((p < (s_raw + rawLen)) && (*p != ','))
        {
            p++;
        }
        if ((p >= (s_raw + rawLen)) || (*p != ','))
        {
            break;
        }
        p++;
        if (*p != '"')
        {
            i = (uint16_t)(p - s_raw);
            continue;
        }
        p++;
        while ((p < (s_raw + rawLen)) && (*p != '"'))
        {
            p++;
        }
        if ((p >= (s_raw + rawLen)) || (*p != '"'))
        {
            break;
        }
        p++;
        if (*p != ',')
        {
            i = (uint16_t)(p - s_raw);
            continue;
        }
        p++;

        len = 0U;
        while ((p < (s_raw + rawLen)) && (*p >= '0') && (*p <= '9'))
        {
            len = (len * 10U) + (unsigned)(*p - '0');
            p++;
        }
        if ((p >= (s_raw + rawLen)) || (*p != ',') || (len == 0U))
        {
            i = (uint16_t)(p - s_raw);
            continue;
        }
        p++;

        pi = (uint16_t)(p - s_raw);
        if (((uint32_t)pi + (uint32_t)len) > (uint32_t)rawLen)
        {
            DebugFramework_Printf("[SHADOW] incomplete need=%u have=%u\n\r",
                                  (unsigned)len, (unsigned)(rawLen - pi));
            break;
        }

        payloadLen = 0U;
        memset(s_payload, 0, sizeof(s_payload));
        for (k = 0U; k < len; k++)
        {
            if ((payloadLen + 1U) < (uint16_t)sizeof(s_payload))
            {
                s_payload[payloadLen++] = s_raw[pi + (uint16_t)k];
                s_payload[payloadLen] = '\0';
            }
        }

        DebugFramework_Printf("[SHADOW] RX len=%u\n\r", (unsigned)payloadLen);
        {
            char preview[81];
            uint16_t n = (payloadLen > 80U) ? 80U : payloadLen;
            uint16_t j;
            for (j = 0U; j < n; j++)
            {
                char c = s_payload[j];
                preview[j] = ((c >= 32) && (c < 127)) ? c : '.';
            }
            preview[n] = '\0';
            DebugFramework_PutsLine(preview);
        }

        if (onJson(s_payload, user) == 0)
        {
            handled = 1;
        }

        i = (uint16_t)(pi + len);
    }

    return handled;
}

/* ---- Public API -------------------------------------------------------- */

/**
 * Drain a few ms of UART junk before TX.
 */
static void shadow_uart_drain(uint32_t ms)
{
    const MqttPort_Interface *port = MqttPort_Get();
    uint32_t t0;

    if (port == NULL)
    {
        return;
    }
    t0 = port->get_tick_ms();
    while ((port->get_tick_ms() - t0) < ms)
    {
        uint8_t b;
        (void)port->uart_receive(&b, 1U, 5U);
    }
}

/**
 * Escape " and \ for AT+MQTTPUB="..." data field (fleet-style).
 */
static int shadow_escape_at_string(const char *in, char *out, uint16_t outMax)
{
    uint16_t oi = 0U;

    if ((in == NULL) || (out == NULL) || (outMax < 2U))
    {
        return -1;
    }

    while (*in != '\0')
    {
        if (((*in == '"') || (*in == '\\')) && ((oi + 2U) < outMax))
        {
            out[oi++] = '\\';
            out[oi++] = *in;
        }
        else if ((oi + 1U) < outMax)
        {
            out[oi++] = *in;
        }
        else
        {
            return -1;
        }
        in++;
    }
    out[oi] = '\0';
    return 0;
}

/**
 * Small JSON publish via AT+MQTTPUB (no '>' — custom PUBRAW was failing).
 */
static int shadow_mqtt_pub_json(const char *topic, const char *json)
{
    char esc[160];
    char cmd[360];
    const MqttPort_Interface *port = MqttPort_Get();
    uint32_t t0;
    uint32_t lastRx;
    uint16_t n = 0U;

    if ((topic == NULL) || (json == NULL) || (port == NULL))
    {
        return -1;
    }
    if (shadow_escape_at_string(json, esc, (uint16_t)sizeof(esc)) != 0)
    {
        return -1;
    }

    shadow_uart_drain(50U);
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTPUB=0,\"%s\",\"%s\",1,0\r\n", topic, esc);
    if (Wifi_SendCommand(cmd) != MQTT_PORT_OK)
    {
        return -1;
    }

    /* Collect short AT response (OK / ERROR), stop early on silence after RX */
    memset(s_raw, 0, sizeof(s_raw));
    t0 = port->get_tick_ms();
    lastRx = t0;
    while ((port->get_tick_ms() - t0) < 2500U)
    {
        uint8_t b;
        if (port->uart_receive(&b, 1U, 5U) == MQTT_PORT_OK)
        {
            if ((n + 1U) < (uint16_t)sizeof(s_raw))
            {
                s_raw[n++] = (char)b;
                s_raw[n] = '\0';
            }
            lastRx = port->get_tick_ms();
            if ((strstr(s_raw, "ERROR") != NULL) ||
                (strstr(s_raw, "OK\r\n") != NULL) ||
                (strstr(s_raw, "OK\n") != NULL))
            {
                /* brief extra for late chars */
                if ((port->get_tick_ms() - lastRx) >= 30U)
                {
                    break;
                }
            }
            continue;
        }
        if ((n > 0U) && ((port->get_tick_ms() - lastRx) >= 80U))
        {
            break;
        }
    }

    if (n == 0U)
    {
        DebugFramework_PutsLine("[SHADOW] MQTTPUB empty RX");
        return -1;
    }
    if (strstr(s_raw, "ERROR") != NULL)
    {
        DebugFramework_PutsLine("[SHADOW] MQTTPUB ERROR");
        return -1;
    }
    if (strstr(s_raw, "OK") == NULL)
    {
        DebugFramework_PutsLine("[SHADOW] MQTTPUB no OK");
        return -1;
    }
    return 0;
}

int MqttShadow_Subscribe(char *buffer, uint16_t bufSize)
{
    (void)bufSize;

    if (buffer == NULL)
    {
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Sub update/delta...");
    if (Wifi_MqttSub2(buffer, MqttIdentity_GetShadowTopicDelta(), 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] Sub delta FAILED");
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Sub get/accepted...");
    if (Wifi_MqttSub2(buffer, MqttIdentity_GetShadowTopicGetAccepted(), 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[SHADOW] Sub get/accepted FAILED");
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Subs OK");
    return 0;
}

int MqttShadow_RequestGet(char *buffer, uint16_t bufSize)
{
    char cmd[220];

    (void)bufSize;
    if (buffer == NULL)
    {
        return -1;
    }

    /*
     * Small payload → AT+MQTTPUB. Do NOT call Wifi_Receive afterward:
     * get/accepted SUBRECV arrives in the same window as OK and must be
     * collected by PollWindow/slurp (fleet-style).
     */
    DebugFramework_PutsLine("[SHADOW] GET shadow...");
    shadow_uart_drain(30U);
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTPUB=0,\"%s\",\"{}\",1,0\r\n",
             MqttIdentity_GetShadowTopicGet());
    if (Wifi_SendCommand(cmd) != MQTT_PORT_OK)
    {
        DebugFramework_PutsLine("[SHADOW] GET TX failed");
        return -1;
    }
    return 0;
}

int MqttShadow_PublishReported(char *buffer, uint16_t bufSize,
                               const char *reportedBody)
{
    char envelope[128];
    int n;

    if ((buffer == NULL) || (reportedBody == NULL) || (bufSize < 128U))
    {
        return -1;
    }

    n = snprintf(envelope, sizeof(envelope),
                 "{\"state\":{\"reported\":%s}}", reportedBody);
    if ((n <= 0) || (n >= (int)sizeof(envelope)))
    {
        return -1;
    }

    DebugFramework_PutsLine("[SHADOW] Publish reported...");
    if (shadow_mqtt_pub_json(MqttIdentity_GetShadowTopicUpdate(), envelope) != 0)
    {
        /* Fallback: telemetry path (Wifi_MqttPubRaw2) — proven on this board */
        DebugFramework_PutsLine("[SHADOW] MQTTPUB failed — try PUBRAW2");
        if (Wifi_MqttPubRaw2(buffer, (char *)MqttIdentity_GetShadowTopicUpdate(),
                             (uint16_t)strlen(envelope), envelope,
                             QOS_1, RTN_0, POLLING_MODE) != FUNC_OK)
        {
            DebugFramework_PutsLine("[SHADOW] reported TX failed");
            return -1;
        }
    }
    return 0;
}

int MqttShadow_Poll(char *buffer, uint16_t bufSize,
                    int (*onJson)(const char *json, void *user),
                    void *user)
{
    (void)buffer;
    (void)bufSize;
    return shadow_poll_internal(MQTT_SHADOW_POLL_MS, onJson, user);
}

int MqttShadow_PollWindow(char *buffer, uint16_t bufSize, uint32_t windowMs,
                          int (*onJson)(const char *json, void *user),
                          void *user)
{
    (void)buffer;
    (void)bufSize;
    if (windowMs < 100U)
    {
        windowMs = 100U;
    }
    return shadow_poll_internal(windowMs, onJson, user);
}

#endif /* EMPA_ESP32_MQTT_AWS && MQTT_SHADOW_ENABLE */
