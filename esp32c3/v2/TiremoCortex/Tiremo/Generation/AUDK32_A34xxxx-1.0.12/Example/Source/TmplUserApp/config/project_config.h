/**
 * @file    project_config.h
 * @brief   Master config header — pulls in all settings from one place
 *
 * Include in application code as:
 *   #include "config/project_config.h"
 *
 * Sub-files (edit these for your deployment):
 *   app_config.h          -> Enabled modules, timing
 *   board_config.h        -> Pins, UART selection
 *   network_config.h      -> WiFi SSID/password, 4G APN
 *   mqtt_device_config.h  -> Broker, client ID, topics, TLS on/off
 */

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

#include "app_config.h"
#include "board_config.h"
#include "network_config.h"
#include "mqtt_device_config.h"

#endif /* PROJECT_CONFIG_H */
