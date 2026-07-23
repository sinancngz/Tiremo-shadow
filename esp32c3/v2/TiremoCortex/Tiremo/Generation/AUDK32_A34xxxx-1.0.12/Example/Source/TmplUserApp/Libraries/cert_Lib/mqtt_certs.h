/**
 * @file    mqtt_certs.h
 * @brief   Shared AWS IoT / broker TLS certificate accessors (ESP32 + SLM320)
 */

#ifndef MQTT_CERTS_H
#define MQTT_CERTS_H

#include "../../config/mqtt_device_config.h"
#include <stddef.h>
#include <stdint.h>

const char *MqttCerts_GetRootCA(void);
const char *MqttCerts_GetClientCert(void);
const char *MqttCerts_GetPrivateKey(void);

size_t MqttCerts_GetRootCALen(void);
size_t MqttCerts_GetClientCertLen(void);
size_t MqttCerts_GetPrivateKeyLen(void);

/** 1 if .cert_flash section contains valid PEM data (not erased). */
uint8_t MqttCerts_HasEmbedded(void);

/** Erase certificate pages from code-flash and clear RAM buffers. */
int MqttCerts_EraseFlash(void);

/** Log RAM buffer + flash section state. */
void MqttCerts_LogStorage(const char *phase);

/** Log flash cert addresses only when data is present (erased = silent). */
void MqttCerts_LogFlashIfPresent(void);

#endif /* MQTT_CERTS_H */
