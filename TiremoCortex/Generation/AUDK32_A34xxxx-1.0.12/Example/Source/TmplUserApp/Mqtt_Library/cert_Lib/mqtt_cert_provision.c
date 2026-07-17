/**
 * @file    mqtt_cert_provision.c
 * @brief   Tiremo fleet provisioning: bootstrap cert → permanent cert
 *
 * Flow (device provisioning.txt):
 *  1. Upload bootstrap PEMs to ESP32
 *  2. (caller) MQTT connect with bootstrap
 *  3. Create cert via $aws/certificates/create/json
 *  4. Register via tiremo-default template
 *  5. Upload permanent cert/key, mark provisioned, MQTT clean
 */

#include "mqtt_cert_provision.h"
#include "mqtt_certs.h"
#include "../../Tiremo/DebugLibrary/debug_framework.h"
#include "../../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS)

#include "../mqtt_core.h"
#include "../mqtt_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLEET_TOKEN_MAX     512U
#define FLEET_CERT_MAX      1408U
#define FLEET_KEY_MAX       1792U
#define FLEET_WAIT_MS       60000U
#define FLEET_MARKER_NS     "tiremo"
#define FLEET_MARKER_KEY    "provisioned"
#define FLEET_MARKER_VAL    "PROV1"

static char s_token[FLEET_TOKEN_MAX];
static char s_permCert[FLEET_CERT_MAX];
static char s_permKey[FLEET_KEY_MAX];

/* ---- JSON string extract (handles \" and \\n escapes) ------------------ */

static int fleet_extract_json_string(const char *json, const char *key,
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

    while (*p != '\0' && oi < (outMax - 1U))
    {
        if (*p == '\\' && p[1] != '\0')
        {
            if (p[1] == 'n')
            {
                out[oi++] = '\n';
                p += 2;
                continue;
            }
            if (p[1] == '"' || p[1] == '\\' || p[1] == '/')
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

/* ---- +MQTTSUBRECV collector: RX-first, parse-second -------------------- */
/*
 * ESP-AT: +MQTTSUBRECV:<link>,"<topic>",<length>,<payload>
 * Critical: never block on printf/parse while ESP is still streaming frames,
 * or the HW FIFO overflows and the final frame (token) is lost.
 */

static int fleet_read_byte(const MqttPort_Interface *port, uint8_t *byte, uint32_t timeoutMs)
{
    uint32_t start = port->get_tick_ms();
    while ((port->get_tick_ms() - start) < timeoutMs)
    {
        if (port->uart_receive(byte, 1U, 5U) == MQTT_PORT_OK)
        {
            return 0;
        }
    }
    return -1;
}

/**
 * Slurp UART into raw[] with almost no processing.
 * Stop after `silenceMs` with no new bytes, once we have seen "+MQTTSUBRECV"
 * (or hard deadline).
 */
static uint16_t fleet_slurp_uart(char *raw, uint16_t rawMax,
                                 uint32_t deadlineMs, uint32_t silenceMs)
{
    const MqttPort_Interface *port = MqttPort_Get();
    uint32_t t0;
    uint32_t lastRx;
    uint16_t n = 0U;
    uint8_t sawSub = 0U;
    uint8_t matchIdx = 0U;
    static const char s_tag[] = "+MQTTSUBRECV:";

    if ((port == NULL) || (raw == NULL) || (rawMax < 16U))
    {
        return 0U;
    }

    memset(raw, 0, rawMax);
    t0 = port->get_tick_ms();
    lastRx = t0;

    while ((port->get_tick_ms() - t0) < deadlineMs)
    {
        uint8_t b;
        if (port->uart_receive(&b, 1U, 5U) == MQTT_PORT_OK)
        {
            if (n + 1U < rawMax)
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
            break; /* burst finished */
        }
    }

    return n;
}

/**
 * Walk raw AT stream, append every SUBRECV payload into out[].
 */
static uint16_t fleet_extract_payloads(const char *raw, uint16_t rawLen,
                                       char *out, uint16_t outMax)
{
    uint16_t i = 0U;
    uint16_t assembled = 0U;
    uint16_t frames = 0U;

    if ((raw == NULL) || (out == NULL) || (outMax < 2U))
    {
        return 0U;
    }
    out[0] = '\0';

    while (i + 13U < rawLen)
    {
        const char *hit;
        const char *p;
        unsigned len = 0U;
        uint16_t pi;
        unsigned k;

        hit = strstr(&raw[i], "+MQTTSUBRECV:");
        if (hit == NULL)
        {
            break;
        }
        i = (uint16_t)(hit - raw);
        p = &raw[i] + 13; /* after +MQTTSUBRECV: */

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
        p++; /* payload starts */

        pi = (uint16_t)(p - raw);
        if (((uint32_t)pi + (uint32_t)len) > (uint32_t)rawLen)
        {
            DebugFramework_Printf(
                "[FLEET] incomplete frame need=%u have=%u\n\r",
                (unsigned)len, (unsigned)(rawLen - pi));
            break;
        }

        for (k = 0U; k < len; k++)
        {
            if (assembled + 1U < outMax)
            {
                out[assembled++] = raw[pi + (uint16_t)k];
                out[assembled] = '\0';
            }
        }
        frames++;
        i = (uint16_t)(pi + len);
    }

    DebugFramework_Printf("[FLEET] parsed frames=%u payload=%u raw=%u\n\r",
                          (unsigned)frames, (unsigned)assembled, (unsigned)rawLen);
    return assembled;
}

static int fleet_wait_subrecv(char *buffer, uint16_t bufSize,
                              const char *mustContain, uint32_t timeoutMs)
{
    /* Use end of caller's buffer as temporary raw scoop if large enough;
     * else static raw (fleet create needs ~4-5KB raw AT stream). */
    static char s_raw[6144];
    uint16_t rawLen;
    uint16_t payloadLen;
    uint32_t silenceMs = 2500U;

    if ((buffer == NULL) || (bufSize < 64U) || (mustContain == NULL))
    {
        return -1;
    }

    (void)timeoutMs;
    /* Slurp up to 45s, but stop 2.5s after last byte once SUBRECV seen */
    rawLen = fleet_slurp_uart(s_raw, (uint16_t)sizeof(s_raw), 45000U, silenceMs);
    payloadLen = fleet_extract_payloads(s_raw, rawLen, buffer, bufSize);

    if (payloadLen == 0U)
    {
        DebugFramework_PutsLine("[FLEET] no SUBRECV payload parsed");
        return -1;
    }

    if (strstr(buffer, mustContain) != NULL)
    {
        DebugFramework_Printf("[FLEET] marker OK payload=%u\n\r", (unsigned)payloadLen);
        return 0;
    }

    DebugFramework_Printf("[FLEET] collect done payload=%u raw=%u\n\r",
                          (unsigned)payloadLen, (unsigned)rawLen);
    if (strstr(buffer, "-----END") != NULL)
    {
        DebugFramework_PutsLine("[FLEET] saw PEM END marker(s)");
    }
    if (strstr(buffer, "privateKey") != NULL)
    {
        DebugFramework_PutsLine("[FLEET] saw privateKey");
    }
    if (strstr(buffer, "certificateOwnershipToken") != NULL)
    {
        DebugFramework_PutsLine("[FLEET] token string present!");
        return 0;
    }
    return -1;
}

static void fleet_uart_drain(uint32_t ms)
{
    const MqttPort_Interface *port = MqttPort_Get();
    uint32_t start;

    if (port == NULL)
    {
        return;
    }
    start = port->get_tick_ms();
    while ((port->get_tick_ms() - start) < ms)
    {
        uint8_t byte;
        (void)port->uart_receive(&byte, 1U, 10U);
    }
}

/**
 * Publish small JSON with AT+MQTTPUB and collect +MQTTSUBRECV payload(s).
 */
static int fleet_pub_and_collect(char *buffer, uint16_t bufSize,
                                 const char *topic, const char *payload,
                                 const char *mustContain, uint32_t timeoutMs)
{
    char cmd[192];
    const MqttPort_Interface *port = MqttPort_Get();

    if ((buffer == NULL) || (topic == NULL) || (payload == NULL) || (port == NULL))
    {
        return -1;
    }

    fleet_uart_drain(100U);
    memset(buffer, 0, bufSize);

    snprintf(cmd, sizeof(cmd), "AT+MQTTPUB=0,\"%s\",\"%s\",1,0\r\n", topic, payload);
    if (Wifi_SendCommand(cmd) != MQTT_PORT_OK)
    {
        DebugFramework_PutsLine("[FLEET] MQTTPUB TX failed");
        return -1;
    }

    /* Accumulate PUB OK and SUBRECV in one continuous listen window. */
    return fleet_wait_subrecv(buffer, bufSize, mustContain, timeoutMs);
}

/* ---- Provisioned marker in ESP32 NVS ----------------------------------- */

static void fleet_at_sync(void)
{
    char resp[64];
    fleet_uart_drain(50U);
    (void)Wifi_SendCommand("AT\r\n");
    memset(resp, 0, sizeof(resp));
    (void)Wifi_Receive(resp, (uint16_t)sizeof(resp), 500U, POLLING_MODE);
}

uint8_t MqttFleet_IsProvisioned(void)
{
    char cmd[96];
    char resp[160];
    MqttPort_Status st;

    fleet_at_sync();

    snprintf(cmd, sizeof(cmd), "AT+SYSMFG=3,\"%s\",\"%s\"\r\n",
             FLEET_MARKER_NS, FLEET_MARKER_KEY);
    st = Wifi_SendCommand(cmd);
    if (st != MQTT_PORT_OK)
    {
        DebugFramework_PutsLine("[FLEET] marker read TX fail");
        return 0U;
    }

    memset(resp, 0, sizeof(resp));
    (void)Wifi_Receive(resp, (uint16_t)sizeof(resp), 3000U, POLLING_MODE);

    if (strstr(resp, "ERROR") != NULL)
    {
        DebugFramework_PutsLine("[FLEET] marker not set (SYSMFG ERROR)");
        return 0U;
    }

    /* Prefer exact marker payload; length field alone is not enough. */
    if (strstr(resp, FLEET_MARKER_VAL) != NULL)
    {
        DebugFramework_PutsLine("[FLEET] marker OK (PROV1)");
        return 1U;
    }

    DebugFramework_PutsLine("[FLEET] marker miss — rsp:");
    DebugFramework_PutsLine(resp[0] != '\0' ? resp : "(empty)");
    return 0U;
}

static int fleet_set_provisioned_marker(char *buffer)
{
    char cmd[96];
    MqttPort_Status checkCmd;
    MqttPort_Status checkRcv;
    const char *val = FLEET_MARKER_VAL;
    uint16_t len = (uint16_t)strlen(val);
    const MqttPort_Interface *port = MqttPort_Get();

    if (port == NULL)
    {
        return -1;
    }

    fleet_at_sync();

    snprintf(cmd, sizeof(cmd), "AT+SYSMFG=2,\"%s\",\"%s\",8,%u\r\n",
             FLEET_MARKER_NS, FLEET_MARKER_KEY, (unsigned)len);
    checkCmd = Wifi_SendCommand(cmd);
    if (checkCmd != MQTT_PORT_OK)
    {
        return -1;
    }

    /* Wait for '>' prompt */
    {
        uint32_t start = port->get_tick_ms();
        uint16_t got = 0U;
        memset(buffer, 0, 64);
        while ((port->get_tick_ms() - start) < 3000U)
        {
            uint8_t b;
            if (port->uart_receive(&b, 1U, 50U) == MQTT_PORT_OK)
            {
                if (got < 63U)
                {
                    buffer[got++] = (char)b;
                    buffer[got] = '\0';
                }
                if (strchr(buffer, '>') != NULL)
                {
                    break;
                }
            }
        }
        if (strchr(buffer, '>') == NULL)
        {
            return -1;
        }
    }

    if (port->uart_transmit((const uint8_t *)val, len, 3000U) != MQTT_PORT_OK)
    {
        return -1;
    }

    checkRcv = Wifi_Receive(buffer, 120U, 5000U, POLLING_MODE);
    if (checkRcv != MQTT_PORT_OK && checkRcv != MQTT_PORT_TIMEOUT)
    {
        return -1;
    }
    if (Wifi_CheckResponse(buffer, "OK\r\n") != RESP_MSG_OK &&
        strstr(buffer, "OK") == NULL)
    {
        return -1;
    }

    /* Verify read-back */
    if (MqttFleet_IsProvisioned() == 0U)
    {
        DebugFramework_PutsLine("[FLEET] WARN: marker write OK but read-back failed");
        return -1;
    }
    return 0;
}

/* ---- Public API -------------------------------------------------------- */

int MqttFleet_UploadBootstrap(char *buffer, uint16_t bufSize)
{
    (void)bufSize;

    if (MqttCerts_HasEmbedded() == 0U)
    {
        DebugFramework_PutsLine("[FLEET] No bootstrap certificates in firmware");
        return -1;
    }

    DebugFramework_PutsLine("[FLEET] Uploading bootstrap certs to ESP32...");
    if (Wifi_MqttCertsUpload2(buffer, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[FLEET] Bootstrap upload failed");
        return -1;
    }
    DebugFramework_PutsLine("[FLEET] Bootstrap upload OK");
    return 0;
}

int MqttFleet_Run(char *buffer, uint16_t bufSize)
{
    char regJson[768];
    int waitRc;

    if ((buffer == NULL) || (bufSize < 1024U))
    {
        return -1;
    }

    memset(s_token, 0, sizeof(s_token));
    memset(s_permCert, 0, sizeof(s_permCert));
    memset(s_permKey, 0, sizeof(s_permKey));

    DebugFramework_PutsLine("[FLEET] Subscribe create accepted/rejected...");
    if (Wifi_MqttSub2(buffer, MQTT_FLEET_CREATE_ACCEPTED, 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[FLEET] Sub create/accepted failed — AT rsp:");
        DebugFramework_PutsLine(buffer[0] != '\0' ? buffer : "(empty)");
        return -1;
    }
    if (Wifi_MqttSub2(buffer, MQTT_FLEET_CREATE_REJECTED, 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[FLEET] Sub create/rejected failed — AT rsp:");
        DebugFramework_PutsLine(buffer[0] != '\0' ? buffer : "(empty)");
        return -1;
    }

    DebugFramework_PutsLine("[FLEET] Request new certificate...");
    waitRc = fleet_pub_and_collect(buffer, bufSize,
                                   MQTT_FLEET_CREATE_TOPIC, "{}",
                                   "certificateOwnershipToken", FLEET_WAIT_MS);
    if (waitRc == -2)
    {
        DebugFramework_PutsLine("[FLEET] Create REJECTED");
        DebugFramework_PutsLine(buffer);
        return -1;
    }
    if (waitRc != 0)
    {
        DebugFramework_PutsLine("[FLEET] Create timeout / no token");
        DebugFramework_Printf("[FLEET] assembled strlen=%u\n\r",
                              (unsigned)strlen(buffer));
        if (strstr(buffer, "certificatePem") != NULL)
        {
            DebugFramework_PutsLine("[FLEET] saw certificatePem in buffer");
        }
        if (strstr(buffer, "privateKey") != NULL)
        {
            DebugFramework_PutsLine("[FLEET] saw privateKey in buffer");
        }
        if (strstr(buffer, "certificateOwnershipToken") != NULL)
        {
            DebugFramework_PutsLine("[FLEET] token present but extract failed?");
        }
        {
            char preview[121];
            uint16_t n = 0U;
            while ((n < 120U) && (buffer[n] != '\0'))
            {
                char c = buffer[n];
                preview[n] = ((c >= 32) && (c < 127)) ? c : '.';
                n++;
            }
            preview[n] = '\0';
            DebugFramework_PutsLine(preview);
        }
        return -1;
    }

    if (fleet_extract_json_string(buffer, "certificateOwnershipToken",
                                  s_token, (uint16_t)sizeof(s_token)) != 0)
    {
        DebugFramework_PutsLine("[FLEET] Parse token failed");
        return -1;
    }
    if (fleet_extract_json_string(buffer, "certificatePem",
                                  s_permCert, (uint16_t)sizeof(s_permCert)) != 0)
    {
        DebugFramework_PutsLine("[FLEET] Parse certificatePem failed");
        return -1;
    }
    if (fleet_extract_json_string(buffer, "privateKey",
                                  s_permKey, (uint16_t)sizeof(s_permKey)) != 0)
    {
        DebugFramework_PutsLine("[FLEET] Parse privateKey failed");
        return -1;
    }
    DebugFramework_PutsLine("[FLEET] Create ACCEPTED — permanent cert received");

    DebugFramework_PutsLine("[FLEET] Subscribe register accepted/rejected...");
    if (Wifi_MqttSub2(buffer, MQTT_FLEET_REGISTER_ACCEPTED, 1, POLLING_MODE) != FUNC_OK ||
        Wifi_MqttSub2(buffer, MQTT_FLEET_REGISTER_REJECTED, 1, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[FLEET] Sub register failed");
        return -1;
    }

#if MQTT_USE_PRINCIPAL
    snprintf(regJson, sizeof(regJson),
             "{\"certificateOwnershipToken\":\"%s\","
             "\"parameters\":{"
             "\"deviceIdentifier\":\"%s\","
             "\"productIdentifier\":\"%s\","
             "\"productSecret\":\"%s\","
             "\"isGateway\":\"%s\","
             "\"principalIdentifier\":\"%s\"}}",
             s_token,
             MQTT_DEVICE_IDENTIFIER,
             MQTT_PRODUCT_IDENTIFIER,
             MQTT_PRODUCT_SECRET,
             MQTT_IS_GATEWAY,
             MQTT_PRINCIPAL_IDENTIFIER);
#else
    snprintf(regJson, sizeof(regJson),
             "{\"certificateOwnershipToken\":\"%s\","
             "\"parameters\":{"
             "\"deviceIdentifier\":\"%s\","
             "\"productIdentifier\":\"%s\","
             "\"productSecret\":\"%s\","
             "\"isGateway\":\"%s\"}}",
             s_token,
             MQTT_DEVICE_IDENTIFIER,
             MQTT_PRODUCT_IDENTIFIER,
             MQTT_PRODUCT_SECRET,
             MQTT_IS_GATEWAY);
#endif

    DebugFramework_PutsLine("[FLEET] Register device...");
    if (Wifi_MqttPubRaw2(buffer, (char *)MQTT_FLEET_REGISTER_TOPIC,
                         (uint16_t)strlen(regJson), regJson,
                         QOS_1, RTN_0, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[FLEET] Register publish failed");
        return -1;
    }

    waitRc = fleet_wait_subrecv(buffer, bufSize, "thingName", FLEET_WAIT_MS);
    if (waitRc == -2)
    {
        DebugFramework_PutsLine("[FLEET] Register REJECTED");
        DebugFramework_PutsLine(buffer);
        return -1;
    }
    if (waitRc != 0)
    {
        DebugFramework_PutsLine("[FLEET] Register timeout");
        return -1;
    }
    DebugFramework_PutsLine("[FLEET] Register ACCEPTED");
    {
        char thing[80];
        if (fleet_extract_json_string(buffer, "thingName", thing, (uint16_t)sizeof(thing)) == 0)
        {
            DebugFramework_PutsLine("[FLEET] thingName:");
            DebugFramework_PutsLine(thing);
        }
    }

    if (MqttCerts_SetPermanent(s_permCert, s_permKey) != 0)
    {
        DebugFramework_PutsLine("[FLEET] SetPermanent failed");
        return -1;
    }

    DebugFramework_PutsLine("[FLEET] Uploading permanent certs to ESP32...");
    if (Wifi_MqttCertsUpload2(buffer, POLLING_MODE) != FUNC_OK)
    {
        DebugFramework_PutsLine("[FLEET] Permanent upload failed");
        return -1;
    }

    if (fleet_set_provisioned_marker(buffer) != 0)
    {
        DebugFramework_PutsLine("[FLEET] WARN: provisioned marker write failed");
    }
    else
    {
        DebugFramework_PutsLine("[FLEET] Provisioned marker saved");
    }

    DebugFramework_PutsLine("[FLEET] Disconnect bootstrap MQTT...");
    (void)Wifi_MqttClean2(buffer, POLLING_MODE);
    flag_mqtt_connect = 0;

    /* Wipe secrets from RAM (still on ESP32 NVS). */
    memset(s_token, 0, sizeof(s_token));
    memset(s_permKey, 0, sizeof(s_permKey));

    DebugFramework_PutsLine("[FLEET] Fleet provisioning complete");
    return 0;
}

int MqttFleet_MarkProvisioned(char *buffer, uint16_t bufSize)
{
    if ((buffer == NULL) || (bufSize < 128U))
    {
        return -1;
    }
    return fleet_set_provisioned_marker(buffer);
}

int MqttCertProv_Run(char *buffer, uint16_t bufSize)
{
    if (MqttFleet_IsProvisioned() != 0U)
    {
        DebugFramework_PutsLine("[FLEET] Already provisioned — skip bootstrap upload");
        return 0;
    }
    return MqttFleet_UploadBootstrap(buffer, bufSize);
}

#else /* !EMPA_ESP32_MQTT_AWS */

uint8_t MqttFleet_IsProvisioned(void) { return 0U; }
int MqttFleet_MarkProvisioned(char *buffer, uint16_t bufSize)
{
    (void)buffer;
    (void)bufSize;
    return 0;
}
int MqttFleet_UploadBootstrap(char *buffer, uint16_t bufSize)
{
    (void)buffer;
    (void)bufSize;
    return 0;
}
int MqttFleet_Run(char *buffer, uint16_t bufSize)
{
    (void)buffer;
    (void)bufSize;
    return 0;
}
int MqttCertProv_Run(char *buffer, uint16_t bufSize)
{
    (void)buffer;
    (void)bufSize;
    return 0;
}

#endif /* EMPA_ESP32_MQTT_AWS */
