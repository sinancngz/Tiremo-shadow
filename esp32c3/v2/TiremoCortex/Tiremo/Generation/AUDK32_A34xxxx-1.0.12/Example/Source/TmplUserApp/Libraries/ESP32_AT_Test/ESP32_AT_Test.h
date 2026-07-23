/**
 *******************************************************************************
 * @file        ESP32_AT_Test.h
 * @author      MQTT Team
 * @brief       Simple AT Test Header
 *******************************************************************************
 */

#ifndef ESP32_AT_TEST_H
#define ESP32_AT_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    ESP32_TEST_OK = 0,
    ESP32_TEST_TIMEOUT,
    ESP32_TEST_ERROR,
    ESP32_TEST_NO_RESPONSE,
    ESP32_TEST_WRONG_RESPONSE
} ESP32_Test_Result_e;

typedef enum
{
    ESP32_TEST_STEP_AT = 0,
    ESP32_TEST_STEP_COMPLETE
} ESP32_Test_Step_e;

/** Configure PA12 and power on ESP32 (call after SysTick is running). */
void ESP32_HW_Init(void);
void ESP32_PowerOn(void);
void ESP32_PowerOff(void);
void ESP32_PowerCycle(void);

/**
 * @brief Simple AT test - Send AT, check if OK received
 * @return 0 if OK, -1 if failed
 */
int ESP32_Simple_AT_Test(void);

/**
 * @brief AT test with PA12 power-cycle recovery on failure
 * @return 0 if OK, -1 if failed after all retries
 */
int ESP32_AT_Test_WithRecovery(void);

// Placeholder functions for compatibility
ESP32_Test_Result_e ESP32_AT_Test_Init(void);
ESP32_Test_Result_e ESP32_AT_Test_BasicAT(void);
ESP32_Test_Result_e ESP32_AT_Test_GetVersion(void);
ESP32_Test_Result_e ESP32_AT_Test_Reset(void);
ESP32_Test_Result_e ESP32_AT_Test_SetStationMode(void);
ESP32_Test_Result_e ESP32_AT_Test_ConnectWiFi(const char* ssid, const char* password);
ESP32_Test_Result_e ESP32_AT_Test_GetIP(void);
ESP32_Test_Result_e ESP32_AT_RunAllTests(const char* ssid, const char* password);
const char* ESP32_AT_Test_GetLastError(void);

#ifdef __cplusplus
}
#endif

#endif /* ESP32_AT_TEST_H */

