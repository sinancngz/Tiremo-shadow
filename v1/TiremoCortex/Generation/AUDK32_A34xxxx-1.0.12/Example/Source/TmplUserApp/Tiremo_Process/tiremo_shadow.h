/**
 * @file    tiremo_shadow.h
 * @brief   Device Shadow app mapping — lightState on/off (all LEDs)
 *
 * Flow: subscribe → get → apply AWS delta only → publish reported.
 * Device does NOT compare desired vs reported.
 */

#ifndef TIREMO_SHADOW_H_
#define TIREMO_SHADOW_H_

#include <stdint.h>
#include "../Config/project_config.h"

#if defined(EMPA_ESP32_MQTT_AWS) && (MQTT_SHADOW_ENABLE != 0)

typedef struct
{
    uint8_t  lightState;   /* 0=false/off, 1=true/on */
    uint8_t  subscribed;
} TiremoShadowState_t;

void TiremoShadow_Init(void);

/**
 * @brief  After MQTT connect: sub + get + poll delta + report if applied.
 */
int TiremoShadow_OnConnected(char *buffer, uint16_t bufSize);

/**
 * @brief  Poll for delta; apply lightState; report applied fields.
 */
void TiremoShadow_Poll(char *buffer, uint16_t bufSize);

/**
 * @brief  Keep draining ESP32 UART for shadow SUBRECV for durationMs.
 *         Replaces blocking DelayMs so async delta is not lost.
 */
void TiremoShadow_ServiceForMs(char *buffer, uint16_t bufSize, uint32_t durationMs);

uint32_t TiremoShadow_GetPublishIntervalMs(void);
const char *TiremoShadow_GetPubTopic(void);
const char *TiremoShadow_GetAlarmTopic(void);
const TiremoShadowState_t *TiremoShadow_GetState(void);

#endif /* EMPA_ESP32_MQTT_AWS && MQTT_SHADOW_ENABLE */

#endif /* TIREMO_SHADOW_H_ */
