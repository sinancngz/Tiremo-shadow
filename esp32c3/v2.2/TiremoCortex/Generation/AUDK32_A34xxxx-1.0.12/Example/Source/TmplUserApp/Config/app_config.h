/**
 * @file    app_config.h
 * @brief   Application mode and timing — Tiremo WiFi MQTT + sensors
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* =========================================================================
 * FEATURE FLAGS
 * ========================================================================= */

//#define EMPA_SENSOR_PROCESS
/* Optional: button-gated UART sensor dump for lab debug. */

#define EMPA_ESP32_MQTT_AWS
/* Tiremo cloud: WiFi + fleet provisioning + mutual TLS MQTT. */

/* =========================================================================
 * TIMING
 * ========================================================================= */

#define APP_PUBLISH_INTERVAL_MS     2000U
#define APP_MQTT_CONNECT_RETRIES    3U
#define APP_MQTT_PUBLISH_RETRIES    3U
/* Publish attempts before WiFi/MQTT recover. */
#define APP_SYSTICK_1MS_DIV         1000U
#define ESP32_AT_POWER_CYCLE_MAX    2U
#define APP_BTN_LONG_PRESS_MS       3000U

#endif /* APP_CONFIG_H */
