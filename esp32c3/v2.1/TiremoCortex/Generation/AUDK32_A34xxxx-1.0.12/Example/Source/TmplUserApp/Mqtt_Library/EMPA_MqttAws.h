/*
 * EMPA_MqttAws.h
 *
 *  Created on: May 19, 2024
 *      Author: bilalk
 */

#ifndef INC_EMPA_MQTTAWS_H_
#define INC_EMPA_MQTTAWS_H_

#include "mqtt_core.h"
#include "../Tiremo_Process/sensor.h"

#include "../Config/app_config.h"
#define MAX_TRY_FUNC  APP_MQTT_CONNECT_RETRIES

/* Connect to MQTT broker (blocking, retries up to MAX_TRY_FUNC) */
/* Returns 0 on success, 1 on failure */
uint8_t MQTT_ConnectBroker(void);

/**
 * @brief  Connect without publishing the hello/telemetry message.
 *         Use for bootstrap fleet sessions (claim cert cannot pub telemetry).
 */
uint8_t MQTT_ConnectBrokerQuiet(void);

/**
 * @brief  Point mqttConfig clientId / topics at MAC-derived identity.
 *         Call after MqttIdentity_InitFromMac().
 */
void MQTT_ApplyRuntimeIdentity(void);

/* Publish sensor data as JSON to the configured topic */
void MQTT_PublishSensorData(const SensorData_t *pData);

/* Publish alarm JSON to MQTT_TOPIC_ALARM */
void MQTT_PublishAlarm(const char *jsonPayload);

/* Legacy function kept for compatibility */
void MY_MqttAwsProcess(void);

#endif /* INC_EMPA_MQTTAWS_H_ */
