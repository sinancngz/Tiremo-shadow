/**
 * @file    mqtt_shadow.h
 * @brief   AWS IoT classic Device Shadow MQTT client (ESP-AT)
 *
 * Device applies AWS-computed delta only — never compares desired vs reported.
 */

#ifndef MQTT_SHADOW_H
#define MQTT_SHADOW_H

#include <stdint.h>
#include "../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)

/**
 * @brief  Subscribe: update/delta, get/accepted, update/accepted (+ rejected warn).
 * @return 0 on success, -1 on failure.
 */
int MqttShadow_Subscribe(char *buffer, uint16_t bufSize);

/**
 * @brief  Publish empty GET to fetch pending delta (if any).
 */
int MqttShadow_RequestGet(char *buffer, uint16_t bufSize);

/**
 * @brief  Publish reported object body, e.g. {"lightState":true}
 */
int MqttShadow_PublishReported(char *buffer, uint16_t bufSize,
                               const char *reportedBody);

/**
 * @brief  Non-blocking-ish poll with default MQTT_SHADOW_POLL_MS window.
 */
int MqttShadow_Poll(char *buffer, uint16_t bufSize,
                    int (*onDeltaJson)(const char *deltaJson, void *user),
                    void *user);

/**
 * @brief  Same as Poll but with explicit listen window (ms).
 *         Use a long continuous window in idle so SUBRECV frames are not split.
 */
int MqttShadow_PollWindow(char *buffer, uint16_t bufSize, uint32_t windowMs,
                          int (*onDeltaJson)(const char *deltaJson, void *user),
                          void *user);

int MqttShadow_ExtractString(const char *json, const char *key,
                             char *out, uint16_t outMax);
int MqttShadow_ExtractInt(const char *json, const char *key, int32_t *out);

/**
 * @brief  Extract JSON boolean: "key":true|false (also accepts 1|0).
 * @return 0 on success (*out = 0 or 1), -1 if missing.
 */
int MqttShadow_ExtractBool(const char *json, const char *key, uint8_t *out);

const char *MqttShadow_FindObject(const char *json, const char *key);

#endif /* EMPA_ESP32_MQTT_AWS && MQTT_SHADOW_ENABLE */

#endif /* MQTT_SHADOW_H */
