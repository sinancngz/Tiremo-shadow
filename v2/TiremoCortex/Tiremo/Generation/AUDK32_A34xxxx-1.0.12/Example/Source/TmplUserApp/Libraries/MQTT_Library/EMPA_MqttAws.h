/*
 * EMPA_MqttAws.h
 *
 *  Created on: May 19, 2024
 *      Author: bilalk
 */

#ifndef INC_EMPA_MQTTAWS_H_
#define INC_EMPA_MQTTAWS_H_

#include "mqtt_core.h"
#include "../Sensor/sensor.h"

#include "../../config/app_config.h"
#define MAX_TRY_FUNC  APP_MQTT_CONNECT_RETRIES

/* Connect to MQTT broker (blocking, retries up to MAX_TRY_FUNC) */
/* Returns 0 on success, 1 on failure */
uint8_t MQTT_ConnectBroker(void);

/* Publish sensor data as JSON to the configured topic */
void MQTT_PublishSensorData(const SensorData_t *pData);

/* Publish alarm JSON to MQTT_TOPIC_ALARM */
void MQTT_PublishAlarm(const char *jsonPayload);

/* Legacy function kept for compatibility */
void MY_MqttAwsProcess(void);

#endif /* INC_EMPA_MQTTAWS_H_ */
