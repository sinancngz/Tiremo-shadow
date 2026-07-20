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
/* When enabled: print sensor data to debug UART (no MQTT publish).
 * When disabled: no terminal output, read-only. */

#define EMPA_ESP32_MQTT_AWS
/* When enabled: WiFi + MQTT connection via ESP32-C3.
 * WiFi settings in network_config.h, broker settings in mqtt_device_config.h */

//#define EMPA_SLM320_4G
/* When enabled: 4G LTE + MQTT connection via MeiG SLM320.
 * APN in network_config.h, broker/TLS in mqtt_device_config.h */

/* Note: ESP32 and SLM320 can be enabled together; both use mqtt_device_config.h.
 * They use different UARTs (UART2 vs UART1). */

/* =========================================================================
 * TIMING
 * ========================================================================= */

#define APP_PUBLISH_INTERVAL_MS     2000U
/* Delay between publish cycles in the main loop (milliseconds).
 * Example: 2000 = send sensor data every 2 seconds. */

#define APP_MQTT_CONNECT_RETRIES    3U
/* Number of MQTT connect retries before giving up. */

#define APP_SYSTICK_1MS_DIV         1000U
/* SysTick divider for 1 ms tick. SystemCoreClock / this value = 1 ms.
 * Usually leave unchanged (1000 = 1 kHz SysTick). */

#define ESP32_AT_POWER_CYCLE_MAX    2U
/* After AT test fails, power-cycle ESP32 (PA12) this many times and retry. */

#define APP_BTN_LONG_PRESS_MS       3000U

#endif /* APP_CONFIG_H */
