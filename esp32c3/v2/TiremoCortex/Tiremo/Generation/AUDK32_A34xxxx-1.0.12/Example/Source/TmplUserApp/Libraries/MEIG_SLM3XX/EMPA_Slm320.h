/**
 * @file    EMPA_Slm320.h
 * @brief   SLM320 4G MQTT service layer (application-facing API)
 */

#ifndef EMPA_SLM320_H
#define EMPA_SLM320_H

#include "slm320.h"
#include "../Sensor/sensor.h"
#include "../../config/app_config.h"

#define SLM320_MAX_TRY_CONNECT   APP_MQTT_CONNECT_RETRIES

uint8_t SLM320_ConnectBroker(void);
/** Power-cycle modem then run full connect (use after link loss / SIM swap). */
uint8_t SLM320_ReconnectBroker(void);
uint8_t SLM320_PublishSensorDataApp(const SensorData_t *pData);
uint8_t SLM320_PublishAlarm(const char *jsonPayload);

#endif /* EMPA_SLM320_H */
