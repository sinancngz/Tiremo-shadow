/**
 * @file    mqtt_shadow.h
 * @brief   AWS IoT classic Device Shadow MQTT client (ESP-AT)
 *
 * Delivers shadow JSON to the app. App applies AWS delta / restores reported.
 */

#ifndef MQTT_SHADOW_H
#define MQTT_SHADOW_H

#include <stdint.h>
#include "../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)

int MqttShadow_Subscribe(char *buffer, uint16_t bufSize);
int MqttShadow_RequestGet(char *buffer, uint16_t bufSize);
int MqttShadow_PublishReported(char *buffer, uint16_t bufSize,
                               const char *reportedBody);

/** @param onJson  Called once per MQTT shadow payload. Return 0 if handled. */
int MqttShadow_Poll(char *buffer, uint16_t bufSize,
                    int (*onJson)(const char *json, void *user),
                    void *user);

int MqttShadow_PollWindow(char *buffer, uint16_t bufSize, uint32_t windowMs,
                          int (*onJson)(const char *json, void *user),
                          void *user);

int MqttShadow_ExtractString(const char *json, const char *key,
                             char *out, uint16_t outMax);
int MqttShadow_ExtractInt(const char *json, const char *key, int32_t *out);
int MqttShadow_ExtractBool(const char *json, const char *key, uint8_t *out);
const char *MqttShadow_FindObject(const char *json, const char *key);

#endif /* EMPA_ESP32_MQTT_AWS && MQTT_SHADOW_ENABLE */

#endif /* MQTT_SHADOW_H */
