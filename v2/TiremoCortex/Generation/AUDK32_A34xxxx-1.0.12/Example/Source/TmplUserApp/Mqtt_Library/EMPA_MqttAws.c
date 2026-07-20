#include "../Config/project_config.h"
#include "EMPA_MqttAws.h"
#include "mqtt_device_identity.h"
#include <string.h>
#include <stdio.h>

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)
#include "../Tiremo_Process/tiremo_shadow.h"
#endif

MQTT_Config mqttConfig = {
    .mqttPacketBuffer = mqttPacketBuffer,
    .mode_wifi = STATION_MODE,
    .OSC_enable = SC_DISABLE,
    .wifiID = WIFI_SSID,
    .wifiPassword = WIFI_PASSWORD,
    .timezone = WIFI_TIMEZONE,
#if MQTT_USE_TLS_CERTS
    .mode_mqtt = MQTT_TLS_4,
#else
    .mode_mqtt = MQTT_TCP,
#endif
    .clientID = MQTT_CLIENT_ID,
    .username = "",
    .mqttPassword = "",
    .keepAlive = MQTT_KEEP_ALIVE,
    .cleanSession = CLS_1,
    .qos = QOS_0,
    .retain = RTN_0,
    .brokerAddress = MQTT_BROKER_HOST,
    .reconnect = 0,
    .subtopic = MQTT_TOPIC_SUB,
    .pubtopic = MQTT_TOPIC_PUB
};

void MQTT_ApplyRuntimeIdentity(void)
{
    const char *clientId = MqttIdentity_GetClientId();
    const char *pub = MqttIdentity_GetTopicPub();
    const char *sub = MqttIdentity_GetTopicSub();

    mqttConfig.clientID = (char *)clientId;
    mqttConfig.deviceId = (char *)MqttIdentity_GetDeviceId();

    memset(mqttConfig.pubtopic, 0, sizeof(mqttConfig.pubtopic));
    memset(mqttConfig.subtopic, 0, sizeof(mqttConfig.subtopic));
    strncpy(mqttConfig.pubtopic, pub, sizeof(mqttConfig.pubtopic) - 1U);
    strncpy(mqttConfig.subtopic, sub, sizeof(mqttConfig.subtopic) - 1U);
}

static uint8_t mqtt_connect_broker_internal(uint8_t publishHello)
{
    uint8_t tryCount = 0;

    while (tryCount < MAX_TRY_FUNC)
    {
        if (MQTT_Init(&mqttConfig) == FUNC_SUCCESSFUL)
        {
            LED_Mqttconnected(1);

            if (publishHello != 0U)
            {
                char msg[64];
                snprintf(msg, sizeof(msg), "%s MQTT Successful", mqttConfig.clientID);
                memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
                Wifi_MqttPubRaw2(mqttPacketBuffer, mqttConfig.pubtopic,
                    (uint16_t)strlen(msg), msg, QOS_0, RTN_0, POLLING_MODE);
                LED_MqttTXBlink();
            }
            return 0;
        }
        tryCount++;
    }

    LED_Mqttconnected(0);
    return 1;
}

uint8_t MQTT_ConnectBroker(void)
{
    return mqtt_connect_broker_internal(1U);
}

uint8_t MQTT_ConnectBrokerQuiet(void)
{
    return mqtt_connect_broker_internal(0U);
}

void MQTT_PublishSensorData(const SensorData_t *pData)
{
    static char jsonBuf[256];
    char *topic = mqttConfig.pubtopic;

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)
    topic = (char *)TiremoShadow_GetPubTopic();
#endif

    Sensor_FormatJSON(pData, jsonBuf, sizeof(jsonBuf));

    memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
    LED_MqttTXBlink();
    Wifi_MqttPubRaw2(mqttPacketBuffer, topic,
        (uint16_t)strlen(jsonBuf), jsonBuf, QOS_0, RTN_0, POLLING_MODE);
}

void MQTT_PublishAlarm(const char *jsonPayload)
{
    char *topic = (char *)MqttIdentity_GetTopicAlarm();

    if (jsonPayload == NULL)
        return;

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)
    topic = (char *)TiremoShadow_GetAlarmTopic();
#endif

    memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
    LED_MqttTXBlink();
    Wifi_MqttPubRaw2(mqttPacketBuffer, topic,
        (uint16_t)strlen(jsonPayload), jsonPayload, QOS_0, RTN_0, POLLING_MODE);
}

void MY_MqttAwsProcess(void)
{
    if (MQTT_ConnectBroker() == 0)
    {
        char msg[64];
        snprintf(msg, sizeof(msg), "%s mqtt successful", mqttConfig.clientID);
        memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
        LED_MqttTXBlink();
        Wifi_MqttPubRaw2(mqttPacketBuffer, mqttConfig.pubtopic,
            (uint16_t)strlen(msg), msg, QOS_0, RTN_0, POLLING_MODE);
    }
}
