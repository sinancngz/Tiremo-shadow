/**
 * @file    mqtt_device_identity.c
 * @brief   Build MQTT client ID and topics from ESP32 STA MAC
 */

#include "../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS)

#include "mqtt_device_identity.h"
#include "mqtt_core.h"
#include "mqtt_port.h"
#include "../Tiremo/DebugLibrary/debug_framework.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ID_DEVICE_MAX   16U
#define ID_CLIENT_MAX   48U
#define ID_TOPIC_MAX    80U
#define ID_SHADOW_MAX   96U

static char s_deviceId[ID_DEVICE_MAX];
static char s_clientId[ID_CLIENT_MAX];
static char s_topicPub[ID_TOPIC_MAX];
static char s_topicSub[ID_TOPIC_MAX];
static char s_topicAlarm[ID_TOPIC_MAX];
static char s_shadowUpdate[ID_SHADOW_MAX];
static char s_shadowDelta[ID_SHADOW_MAX];
static char s_shadowGet[ID_SHADOW_MAX];
static char s_shadowGetAccepted[ID_SHADOW_MAX];
static uint8_t s_ready;

static char id_to_lower_hex(char c)
{
    if ((c >= 'A') && (c <= 'F'))
    {
        return (char)(c - 'A' + 'a');
    }
    return c;
}

static int identity_parse_mac_hex12(const char *resp, char out12[13])
{
    const char *p;
    uint8_t n = 0U;

    if ((resp == NULL) || (out12 == NULL))
    {
        return -1;
    }

    p = strstr(resp, "+CIPSTAMAC:\"");
    if (p == NULL)
    {
        return -1;
    }
    p += 12;

    while ((*p != '\0') && (*p != '"') && (n < 12U))
    {
        if (isxdigit((unsigned char)*p) != 0)
        {
            out12[n++] = id_to_lower_hex(*p);
        }
        p++;
    }
    out12[n] = '\0';
    return (n == 12U) ? 0 : -1;
}

static int identity_read_mac_hex12(char *scratch, uint16_t scratchSize, char out12[13])
{
    uint16_t recvLen;
    MqttPort_Status st;

    if ((scratch == NULL) || (scratchSize < 96U) || (out12 == NULL))
    {
        return -1;
    }

    memset(scratch, 0, scratchSize);
    if (Wifi_SendCommand("AT+CIPSTAMAC?\r\n") != MQTT_PORT_OK)
    {
        return -1;
    }

    recvLen = (scratchSize > WIFI_FUNCS_STD_BUFF_SIZE)
                  ? (uint16_t)WIFI_FUNCS_STD_BUFF_SIZE
                  : scratchSize;
    st = Wifi_Receive(scratch, recvLen, 2000U, POLLING_MODE);
    if ((st != MQTT_PORT_OK) && (st != MQTT_PORT_TIMEOUT))
    {
        return -1;
    }

    return identity_parse_mac_hex12(scratch, out12);
}

static void identity_build_strings(void)
{
    snprintf(s_clientId, sizeof(s_clientId), "%s_%s",
             MQTT_PRODUCT_IDENTIFIER, s_deviceId);

#if MQTT_USE_PRINCIPAL
    snprintf(s_topicPub, sizeof(s_topicPub),
             "pub/%s/%s/%s/telemetry",
             MQTT_PRODUCT_IDENTIFIER, MQTT_PRINCIPAL_IDENTIFIER, s_deviceId);
    snprintf(s_topicAlarm, sizeof(s_topicAlarm),
             "pub/%s/%s/%s/alarm",
             MQTT_PRODUCT_IDENTIFIER, MQTT_PRINCIPAL_IDENTIFIER, s_deviceId);
    snprintf(s_topicSub, sizeof(s_topicSub),
             "sub/%s/%s/%s/telemetry",
             MQTT_PRODUCT_IDENTIFIER, MQTT_PRINCIPAL_IDENTIFIER, s_deviceId);
#else
    snprintf(s_topicPub, sizeof(s_topicPub),
             "pub/%s/%s/telemetry",
             MQTT_PRODUCT_IDENTIFIER, s_deviceId);
    snprintf(s_topicAlarm, sizeof(s_topicAlarm),
             "pub/%s/%s/alarm",
             MQTT_PRODUCT_IDENTIFIER, s_deviceId);
    snprintf(s_topicSub, sizeof(s_topicSub),
             "sub/%s/%s/telemetry",
             MQTT_PRODUCT_IDENTIFIER, s_deviceId);
#endif

    snprintf(s_shadowUpdate, sizeof(s_shadowUpdate),
             "$aws/things/%s/shadow/update", s_clientId);
    snprintf(s_shadowDelta, sizeof(s_shadowDelta),
             "$aws/things/%s/shadow/update/delta", s_clientId);
    snprintf(s_shadowGet, sizeof(s_shadowGet),
             "$aws/things/%s/shadow/get", s_clientId);
    snprintf(s_shadowGetAccepted, sizeof(s_shadowGetAccepted),
             "$aws/things/%s/shadow/get/accepted", s_clientId);
}

int MqttIdentity_InitFromMac(char *scratch, uint16_t scratchSize)
{
    char macHex[13];
    int rc;

    s_ready = 0U;
    memset(s_deviceId, 0, sizeof(s_deviceId));
    memset(macHex, 0, sizeof(macHex));

    if ((scratch == NULL) || (scratchSize < 96U))
    {
        return -1;
    }

    rc = identity_read_mac_hex12(scratch, scratchSize, macHex);
    if (rc == 0)
    {
        strncpy(s_deviceId, macHex, sizeof(s_deviceId) - 1U);
        DebugFramework_PutsLine("[ID] deviceIdentifier from MAC:");
        DebugFramework_PutsLine(s_deviceId);
    }
    else
    {
        strncpy(s_deviceId, MQTT_DEVICE_IDENTIFIER, sizeof(s_deviceId) - 1U);
        DebugFramework_PutsLine("[ID] MAC read failed — fallback deviceIdentifier:");
        DebugFramework_PutsLine(s_deviceId);
    }

    identity_build_strings();
    s_ready = 1U;

    DebugFramework_PutsLine("[ID] clientId:");
    DebugFramework_PutsLine(s_clientId);
    return 0;
}

uint8_t MqttIdentity_IsReady(void)
{
    return s_ready;
}

const char *MqttIdentity_GetDeviceId(void)
{
    return (s_ready != 0U) ? s_deviceId : MQTT_DEVICE_IDENTIFIER;
}

const char *MqttIdentity_GetClientId(void)
{
    return (s_ready != 0U) ? s_clientId : MQTT_CLIENT_ID;
}

const char *MqttIdentity_GetTopicPub(void)
{
    return (s_ready != 0U) ? s_topicPub : MQTT_TOPIC_PUB;
}

const char *MqttIdentity_GetTopicSub(void)
{
    return (s_ready != 0U) ? s_topicSub : MQTT_TOPIC_SUB;
}

const char *MqttIdentity_GetTopicAlarm(void)
{
    return (s_ready != 0U) ? s_topicAlarm : MQTT_TOPIC_ALARM;
}

const char *MqttIdentity_GetShadowTopicUpdate(void)
{
    return (s_ready != 0U) ? s_shadowUpdate : MQTT_SHADOW_TOPIC_UPDATE;
}

const char *MqttIdentity_GetShadowTopicDelta(void)
{
    return (s_ready != 0U) ? s_shadowDelta : MQTT_SHADOW_TOPIC_DELTA;
}

const char *MqttIdentity_GetShadowTopicGet(void)
{
    return (s_ready != 0U) ? s_shadowGet : MQTT_SHADOW_TOPIC_GET;
}

const char *MqttIdentity_GetShadowTopicGetAccepted(void)
{
    return (s_ready != 0U) ? s_shadowGetAccepted : MQTT_SHADOW_TOPIC_GET_ACCEPTED;
}

#endif /* EMPA_ESP32_MQTT_AWS */
