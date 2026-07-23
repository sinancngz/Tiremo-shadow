/**
 * @file    network_config.h
 * @brief   WiFi (ESP32) settings for Tiremo MQTT
 */

#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

/* =========================================================================
 * WiFi — ESP32 station join (AT+CWJAP)
 * ========================================================================= */

#define WIFI_SSID                   "EMPA_Arge"
#define WIFI_PASSWORD               "Emp@Arg2024!"
#define WIFI_TIMEZONE               3
/* UTC offset hours (Turkey = 3). Required for TLS certificate date checks. */

#endif /* NETWORK_CONFIG_H */
