/**
 * @file    mqtt_device_config.h
 * @brief   Tiremo broker identity, fleet provisioning, shadow (mutual TLS only)
 */

#ifndef MQTT_DEVICE_CONFIG_H
#define MQTT_DEVICE_CONFIG_H

/* =========================================================================
 * FLEET PROVISIONING IDENTITY (Tiremo platform)
 * serial = {productIdentifier}_{deviceIdentifier}
 *
 * Runtime: deviceIdentifier is taken from ESP32 STA MAC (12 hex chars,
 * lowercase, no colons) via MqttIdentity_InitFromMac().
 * MQTT_DEVICE_IDENTIFIER below is only a fallback if MAC read fails.
 * ========================================================================= */

#define MQTT_PRODUCT_IDENTIFIER     "lightingomj8"
#define MQTT_DEVICE_IDENTIFIER      "30eda080630c"  /* fallback only */
#define MQTT_PRODUCT_SECRET         "9712901b-d328-4256-8ff8-7cbe1cdba50e"
#define MQTT_IS_GATEWAY             "false"
/* Set to 1 and fill MQTT_PRINCIPAL_IDENTIFIER to use principal-scoped topics. */
#define MQTT_USE_PRINCIPAL          0
#define MQTT_PRINCIPAL_IDENTIFIER   ""

#define MQTT_CLIENT_ID              MQTT_PRODUCT_IDENTIFIER "_" MQTT_DEVICE_IDENTIFIER

/* Legacy aliases (older code paths). */
#define MQTT_USER_ID                MQTT_PRODUCT_IDENTIFIER
#define MQTT_DEVICE_NAME            MQTT_DEVICE_IDENTIFIER

/* =========================================================================
 * BROKER — Tiremo only (mutual TLS + fleet provisioning)
 * Plain TCP / WebSocket / cert-less TLS are not supported.
 * ========================================================================= */

#define MQTT_BROKER_HOST            "iot.tiremo.ai"
#define MQTT_BROKER_PORT            8883U

#ifndef MQTT_BROKER_ADDRESS
#define MQTT_BROKER_ADDRESS         MQTT_BROKER_HOST
#endif

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
 * DEVICE SHADOW (AWS IoT classic shadow)
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

#define MQTT_SHADOW_POLL_MS             800U
#define MQTT_SHADOW_GET_WAIT_MS         5000U
#define MQTT_SHADOW_IDLE_LISTEN_MS      2000U
#define MQTT_SHADOW_SUBRECV_SILENCE_MS  1500U

/* =========================================================================
 * TLS — always on (bootstrap PEM → fleet → permanent device cert on ESP NVS)
 *   Bootstrap: certificates/mqtt_certificate.inc + mqtt_private.inc
 *   Root CA:   certificates/mqtt_rootCA.inc
 * ========================================================================= */

#endif /* MQTT_DEVICE_CONFIG_H */
