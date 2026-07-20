/**
 * @file    mqtt_device_config.h
 * @brief   MQTT broker, device identity, TLS, and fleet provisioning settings
 */

#ifndef MQTT_DEVICE_CONFIG_H
#define MQTT_DEVICE_CONFIG_H

/* =========================================================================
 * FLEET PROVISIONING IDENTITY (Tiremo platform)
 * serial = {productIdentifier}_{deviceIdentifier}
 * ========================================================================= */

#define MQTT_PRODUCT_IDENTIFIER     "lightninenwh"
#define MQTT_DEVICE_IDENTIFIER      "e83dc160639c"
#define MQTT_PRODUCT_SECRET         "0cebd880-8964-4b0a-b218-5a66fe5d632e"
#define MQTT_IS_GATEWAY             "false"
/* Set to 1 and fill MQTT_PRINCIPAL_IDENTIFIER to use principal-scoped topics. */
#define MQTT_USE_PRINCIPAL          0
#define MQTT_PRINCIPAL_IDENTIFIER   ""

#define MQTT_CLIENT_ID              MQTT_PRODUCT_IDENTIFIER "_" MQTT_DEVICE_IDENTIFIER
/* Example: lightninenwh_e83dc160639c */

/* Legacy aliases (older code paths). */
#define MQTT_USER_ID                MQTT_PRODUCT_IDENTIFIER
#define MQTT_DEVICE_NAME            MQTT_DEVICE_IDENTIFIER

/* =========================================================================
 * BROKER
 * ========================================================================= */

#define MQTT_BROKER_HOST            "iot.tiremo.ai"

#if MQTT_USE_PRINCIPAL
#define MQTT_TOPIC_PUB              "pub/" MQTT_PRODUCT_IDENTIFIER "/" MQTT_PRINCIPAL_IDENTIFIER "/" MQTT_DEVICE_IDENTIFIER "/telemetry"
#define MQTT_TOPIC_ALARM            "pub/" MQTT_PRODUCT_IDENTIFIER "/" MQTT_PRINCIPAL_IDENTIFIER "/" MQTT_DEVICE_IDENTIFIER "/alarm"
#define MQTT_TOPIC_SUB              "sub/" MQTT_PRODUCT_IDENTIFIER "/" MQTT_PRINCIPAL_IDENTIFIER "/" MQTT_DEVICE_IDENTIFIER "/telemetry"
#else
#define MQTT_TOPIC_PUB              "pub/" MQTT_PRODUCT_IDENTIFIER "/" MQTT_DEVICE_IDENTIFIER "/telemetry"
#define MQTT_TOPIC_ALARM            "pub/" MQTT_PRODUCT_IDENTIFIER "/" MQTT_DEVICE_IDENTIFIER "/alarm"
#define MQTT_TOPIC_SUB              "sub/" MQTT_PRODUCT_IDENTIFIER "/" MQTT_DEVICE_IDENTIFIER "/telemetry"
#endif

#define MQTT_KEEP_ALIVE             60

/* Fleet provisioning MQTT topics (AWS IoT / Tiremo). */
#define MQTT_FLEET_CREATE_TOPIC         "$aws/certificates/create/json"
#define MQTT_FLEET_CREATE_ACCEPTED      "$aws/certificates/create/json/accepted"
#define MQTT_FLEET_CREATE_REJECTED      "$aws/certificates/create/json/rejected"
#define MQTT_FLEET_REGISTER_TOPIC       "$aws/provisioning-templates/tiremo-default/provision/json"
#define MQTT_FLEET_REGISTER_ACCEPTED    "$aws/provisioning-templates/tiremo-default/provision/json/accepted"
#define MQTT_FLEET_REGISTER_REJECTED    "$aws/provisioning-templates/tiremo-default/provision/json/rejected"

/* =========================================================================
 * DEVICE SHADOW (AWS IoT classic shadow — Device State Controller)
 * Thing name = MQTT_CLIENT_ID (serial).
 * ========================================================================= */

#ifndef MQTT_SHADOW_ENABLE
#define MQTT_SHADOW_ENABLE              1
#endif

#define MQTT_SHADOW_THING_NAME          MQTT_CLIENT_ID

#define MQTT_SHADOW_TOPIC_UPDATE         "$aws/things/" MQTT_SHADOW_THING_NAME "/shadow/update"
#define MQTT_SHADOW_TOPIC_DELTA         "$aws/things/" MQTT_SHADOW_THING_NAME "/shadow/update/delta"
#define MQTT_SHADOW_TOPIC_UPD_ACCEPTED  "$aws/things/" MQTT_SHADOW_THING_NAME "/shadow/update/accepted"
#define MQTT_SHADOW_TOPIC_UPD_REJECTED  "$aws/things/" MQTT_SHADOW_THING_NAME "/shadow/update/rejected"
#define MQTT_SHADOW_TOPIC_GET           "$aws/things/" MQTT_SHADOW_THING_NAME "/shadow/get"
#define MQTT_SHADOW_TOPIC_GET_ACCEPTED  "$aws/things/" MQTT_SHADOW_THING_NAME "/shadow/get/accepted"
#define MQTT_SHADOW_TOPIC_GET_REJECTED  "$aws/things/" MQTT_SHADOW_THING_NAME "/shadow/get/rejected"

/* Short UART listen after publish / between cycles (ms). */
#define MQTT_SHADOW_POLL_MS             800U
/* After shadow/get, keep listening this long for get/accepted or delta. */
#define MQTT_SHADOW_GET_WAIT_MS         5000U
/* Idle gap between telemetry: one continuous UART listen (do not split frames). */
#define MQTT_SHADOW_IDLE_LISTEN_MS      2000U
/* After +MQTTSUBRECV seen, wait this long with no bytes before parsing. */
#define MQTT_SHADOW_SUBRECV_SILENCE_MS  1500U

/* =========================================================================
 * TLS CERTIFICATES
 *   Bootstrap PEM: certificates/mqtt_certificate.inc + mqtt_private.inc
 *   Root CA:       certificates/mqtt_rootCA.inc
 *   Permanent certs are written to ESP32 NVS during fleet provisioning.
 * ========================================================================= */

#define MQTT_USE_TLS_CERTS          1

#if MQTT_USE_TLS_CERTS
#define MQTT_BROKER_PORT            8883U
#else
#define MQTT_BROKER_PORT            1883U
#endif

#ifndef MQTT_BROKER_ADDRESS
#define MQTT_BROKER_ADDRESS         MQTT_BROKER_HOST
#endif

#endif /* MQTT_DEVICE_CONFIG_H */
