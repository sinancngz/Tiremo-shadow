/**
 * @file    mqtt_cert_provision.h
 * @brief   Tiremo AWS fleet provisioning (bootstrap → permanent cert)
 */

#ifndef MQTT_CERT_PROVISION_H
#define MQTT_CERT_PROVISION_H

#include <stdint.h>

/**
 * @brief  1 if ESP32 marker matches the current MAC-derived deviceIdentifier.
 */
uint8_t MqttFleet_IsProvisioned(void);

/**
 * @brief  Save current deviceIdentifier as provisioned marker in ESP32 NVS.
 * @return 0 on success, -1 on failure.
 */
int MqttFleet_MarkProvisioned(char *buffer, uint16_t bufSize);

/**
 * @brief  Upload bootstrap RootCA/cert/key from firmware to ESP32 NVS.
 * @return 0 on success, -1 on failure.
 */
int MqttFleet_UploadBootstrap(char *buffer, uint16_t bufSize);

/**
 * @brief  Run fleet create + register while MQTT is already connected
 *         with the bootstrap certificate. Saves permanent cert to ESP32,
 *         sets provisioned marker, disconnects MQTT.
 * @return 0 on success, -1 on failure.
 */
int MqttFleet_Run(char *buffer, uint16_t bufSize);

/**
 * @brief  Boot helper: if not provisioned, upload bootstrap.
 *         (Fleet exchange runs after first MQTT connect.)
 * @return 0 on success, -1 on failure.
 */
int MqttCertProv_Run(char *buffer, uint16_t bufSize);

#endif /* MQTT_CERT_PROVISION_H */
