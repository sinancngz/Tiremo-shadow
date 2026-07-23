/**
 * @file    mqtt_device_identity.h
 * @brief   Runtime device identity from ESP32 STA MAC
 *
 * serial / clientId = {productIdentifier}_{macHex12}
 * Topics and shadow thing name follow the same deviceIdentifier.
 */

#ifndef MQTT_DEVICE_IDENTITY_H
#define MQTT_DEVICE_IDENTITY_H

#include <stdint.h>

/**
 * @brief  Read ESP32 STA MAC, build deviceId / clientId / topics.
 *         On MAC failure falls back to MQTT_DEVICE_IDENTIFIER from config.
 * @param  scratch   Scratch buffer for AT response (min ~96 bytes)
 * @param  scratchSize
 * @return 0 on success (MAC or fallback), -1 if scratch invalid
 */
int MqttIdentity_InitFromMac(char *scratch, uint16_t scratchSize);

/** 1 after a successful InitFromMac (including fallback). */
uint8_t MqttIdentity_IsReady(void);

const char *MqttIdentity_GetDeviceId(void);
const char *MqttIdentity_GetClientId(void);
const char *MqttIdentity_GetTopicPub(void);
const char *MqttIdentity_GetTopicSub(void);
const char *MqttIdentity_GetTopicAlarm(void);

const char *MqttIdentity_GetShadowTopicUpdate(void);
const char *MqttIdentity_GetShadowTopicDelta(void);
const char *MqttIdentity_GetShadowTopicGet(void);
const char *MqttIdentity_GetShadowTopicGetAccepted(void);

#endif /* MQTT_DEVICE_IDENTITY_H */
