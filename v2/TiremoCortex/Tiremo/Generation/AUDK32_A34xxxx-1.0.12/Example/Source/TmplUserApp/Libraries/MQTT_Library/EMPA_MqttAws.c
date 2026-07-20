#include "../../config/project_config.h"
#include "EMPA_MqttAws.h"
#include <string.h>
#include <stdio.h>

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

uint8_t MQTT_ConnectBroker(void)
{
    uint8_t tryCount = 0;

    while (tryCount < MAX_TRY_FUNC)
    {
        if (MQTT_Init(&mqttConfig) == FUNC_SUCCESSFUL)
        {
            LED_Mqttconnected(1);

            char msg[64];
            snprintf(msg, sizeof(msg), "%s MQTT Successful", mqttConfig.clientID);
            memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
            Wifi_MqttPubRaw2(mqttPacketBuffer, mqttConfig.pubtopic,
                (uint16_t)strlen(msg), msg, QOS_0, RTN_0, POLLING_MODE);
            LED_MqttTXBlink();
            return 0;
        }
        tryCount++;
    }

    LED_Mqttconnected(0);
    return 1;
}

void MQTT_PublishSensorData(const SensorData_t *pData)
{
    static char jsonBuf[256];

    Sensor_FormatJSON(pData, jsonBuf, sizeof(jsonBuf));

    memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
    LED_MqttTXBlink();
    Wifi_MqttPubRaw2(mqttPacketBuffer, mqttConfig.pubtopic,
        (uint16_t)strlen(jsonBuf), jsonBuf, QOS_0, RTN_0, POLLING_MODE);
}

void MQTT_PublishAlarm(const char *jsonPayload)
{
    if (jsonPayload == NULL)
        return;

    memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
    LED_MqttTXBlink();
    Wifi_MqttPubRaw2(mqttPacketBuffer, MQTT_TOPIC_ALARM,
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
