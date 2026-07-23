/**
 *******************************************************************************
 * @file        tiremo_app_net.h
 * @brief       Optional ESP32 WiFi + MQTT process helpers
 ******************************************************************************/

#ifndef TIREMO_APP_NET_H_
#define TIREMO_APP_NET_H_

#include <stdint.h>
#include "sensor.h"
#include "sensor_alarm.h"

#if defined(EMPA_ESP32_MQTT_AWS)

/**
 * @brief  ESP32 AT bring-up, MQTT port, TLS certs, broker connect.
 * @return 1 if MQTT connected, 0 otherwise.
 */
uint8_t TiremoAppNet_InitAndConnect(void);

/**
 * @brief       Publish telemetry; reconnect if needed.
 * @param[in]   pData           Sensor sample
 * @param[in,out] pConnected    Connection flag (updated on reconnect)
 * @param[in,out] pDataCount    Publish counter
 */
void TiremoAppNet_PublishCycle(const SensorData_t *pData,
                               uint8_t *pConnected,
                               int *pDataCount);

/**
 * @brief  Edge-triggered alarms: always print on UART; publish if MQTT up.
 */
void TiremoAppNet_PublishAlarms(const SensorData_t *pData,
                                const SensorAlarmType_t *alarms,
                                uint8_t alarmCount,
                                uint8_t connected);

/**
 * @brief  Shadow-aware publish interval (ms). Falls back to APP_PUBLISH_INTERVAL_MS.
 */
uint32_t TiremoAppNet_GetPublishIntervalMs(void);

/**
 * @brief  Idle between telemetry publishes: drain shadow UART (no blind DelayMs).
 */
void TiremoAppNet_IdleService(uint8_t connected, uint32_t durationMs);

#endif /* EMPA_ESP32_MQTT_AWS */

#endif /* TIREMO_APP_NET_H_ */
