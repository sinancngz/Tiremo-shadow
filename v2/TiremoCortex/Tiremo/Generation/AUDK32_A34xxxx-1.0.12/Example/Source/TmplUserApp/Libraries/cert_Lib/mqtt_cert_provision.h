/**
 * @file    mqtt_cert_provision.h
 * @brief   ABOV -> modem certificate upload (ESP32 / SLM320) + flash wipe
 */

#ifndef MQTT_CERT_PROVISION_H
#define MQTT_CERT_PROVISION_H

#include <stdint.h>

/**
 * If ABOV flash has certs: upload to all enabled modems (ESP32, SLM320),
 * wait for long button press, erase ABOV .cert_flash pages.
 * buffer/bufSize used by ESP32 path only (may be NULL for SLM320-only).
 */
int MqttCertProv_Run(char *buffer, uint16_t bufSize);

#endif /* MQTT_CERT_PROVISION_H */
