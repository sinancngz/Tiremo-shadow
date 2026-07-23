/**
 * @file    app_config.h
 * @brief   Application mode and timing settings
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* =========================================================================
 * FEATURE FLAGS — enable features by uncommenting
 * ========================================================================= */

//#define EMPA_SENSOR_PROCESS
/* When enabled: print sensor data to debug UART (button start/stop).
 * When disabled with MQTT only: silent Sensor_ReadOnly(). */

#define EMPA_ESP32_MQTT_AWS
/* When enabled: WiFi + MQTT connection via ESP32-C3.
 * WiFi settings in network_config.h, broker settings in mqtt_device_config.h */

/* SLM320 4G / GNSS are NOT part of this TiremoCortex example.
 * Cellular modem stack is omitted; use EMPA_ESP32_MQTT_AWS for cloud publish. */

/* =========================================================================
 * TIMING
 * ========================================================================= */

#define APP_PUBLISH_INTERVAL_MS     2000U
/* Delay between publish / sensor cycles in the main loop (milliseconds). */

#define APP_MQTT_CONNECT_RETRIES    3U
/* Number of MQTT connect retries before giving up. */

#define APP_SYSTICK_1MS_DIV         1000U
/* SysTick divider for 1 ms tick. SystemCoreClock / this value = 1 ms. */

#define ESP32_AT_POWER_CYCLE_MAX    2U
/* After AT test fails, power-cycle ESP32 (PA12) this many times and retry. */

#define APP_BTN_LONG_PRESS_MS       3000U

#endif /* APP_CONFIG_H */
