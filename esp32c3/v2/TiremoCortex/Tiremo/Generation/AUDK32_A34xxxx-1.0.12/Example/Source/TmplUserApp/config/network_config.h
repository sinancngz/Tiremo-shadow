/**
 * @file    network_config.h
 * @brief   WiFi (ESP32) and cellular (SLM320) network settings
 */

#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

/* =========================================================================
 * WiFi — used only when EMPA_ESP32_MQTT_AWS is enabled
 * ========================================================================= */

#define WIFI_SSID                   "EMPA_Arge"
/* WiFi network name (SSID).
 * ESP32 connects via AT+CWJAP. */

#define WIFI_PASSWORD               "Emp@Arg2024!"
/* WiFi password. Use empty string "" for open networks. */

#define WIFI_TIMEZONE               3
/* UTC offset in hours for NTP time sync.
 * Use 3 for Turkey. Correct time is required for TLS certificate date checks. */

/* =========================================================================
 * Cellular (4G) — used when EMPA_SLM320_4G is enabled
 * ========================================================================= */

#define CELLULAR_APN                "internet"
/* APN name for your SIM operator.
 * "internet" works for most Turkish operators. */

#define CELLULAR_APN_USER           ""
/* APN username. Leave empty if PAP/CHAP is not required. */

#define CELLULAR_APN_PASS           ""
/* APN password. Leave empty when CELLULAR_APN_AUTH = 0. */

#define CELLULAR_APN_AUTH           0
/* APN authentication type.
 *   0 = none (PAP/CHAP disabled) — do not change unless required
 *   1 = PAP, 2 = CHAP (if operator requires) */

#define CELLULAR_PDP_CONTEXT        1
/* PDP context number (for AT+QICSGP and AT+QIACT).
 * Usually 1 on SLM320. */

#endif /* NETWORK_CONFIG_H */
