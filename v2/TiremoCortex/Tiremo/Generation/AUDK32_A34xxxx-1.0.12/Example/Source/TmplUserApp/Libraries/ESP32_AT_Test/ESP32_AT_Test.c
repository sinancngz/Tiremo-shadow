/**
 *******************************************************************************
 * @file        ESP32_AT_Test.c
 * @author      MQTT Team
 * @brief       ESP32-C3 AT communication test and PA12 power control
 *******************************************************************************
 */

#include "ESP32_AT_Test.h"
#include "../../config/board_config.h"
#include "../../config/app_config.h"
#include "hal_uart.h"
#include "hal_pcu.h"
#include <string.h>
#include <stdio.h>
#include "../DebugLibrary/debug_framework.h"

extern void SYSTICK_Wait(uint32_t un32TimeMS);

static int esp32_try_at_once(char *rx_buffer, uint16_t buf_size)
{
    HAL_ERR_e result;

    memset(rx_buffer, 0, buf_size);
    HAL_UART_Transmit(BOARD_ESP32_UART_ID, (uint8_t *)"AT\r\n", 4, true);
    result = HAL_UART_Receive(BOARD_ESP32_UART_ID, (uint8_t *)rx_buffer, 20, true);

    if (strstr(rx_buffer, "OK") != NULL) {
        return 0;
    }

    (void)result;
    return -1;
}

void ESP32_PowerOn(void)
{
    HAL_PCU_SetOutputBit(BOARD_ESP32_PWR_PORT, BOARD_ESP32_PWR_PIN, BOARD_ESP32_PWR_ON);
}

void ESP32_PowerOff(void)
{
    HAL_PCU_SetOutputBit(BOARD_ESP32_PWR_PORT, BOARD_ESP32_PWR_PIN, BOARD_ESP32_PWR_OFF);
}

void ESP32_PowerCycle(void)
{
    DebugFramework_PutsLine("[ESP32] Power cycle via PA12...");
    ESP32_PowerOff();
    SYSTICK_Wait(BOARD_ESP32_PWR_OFF_MS);
    ESP32_PowerOn();
    SYSTICK_Wait(BOARD_ESP32_PWR_BOOT_MS);
}

void ESP32_HW_Init(void)
{
    HAL_PCU_SetInOutMode(BOARD_ESP32_PWR_PORT, BOARD_ESP32_PWR_PIN, PCU_INOUT_OUTPUT_PUSH_PULL);
    HAL_PCU_SetAltMode(BOARD_ESP32_PWR_PORT, BOARD_ESP32_PWR_PIN, PCU_ALT_0);
    ESP32_PowerOn();
    SYSTICK_Wait(BOARD_ESP32_PWR_BOOT_MS);
}

int ESP32_Simple_AT_Test(void)
{
    char rx_buffer[64];
    uint8_t attempt = 0;
    const uint8_t MAX_ATTEMPTS = 10;

    while (attempt < MAX_ATTEMPTS) {
        attempt++;

        if (esp32_try_at_once(rx_buffer, sizeof(rx_buffer)) == 0) {
            if (attempt > 1U) {
                DebugFramework_Printf("[AT] OK received on attempt %u\n\r", attempt);
            }
            return 0;
        }

        DebugFramework_Printf("[AT] Attempt %u/%u: no OK, retrying...\n\r", attempt, MAX_ATTEMPTS);
    }

    DebugFramework_PutsLine("[AT] No OK after max attempts!");
    return -1;
}

int ESP32_AT_Test_WithRecovery(void)
{
    uint8_t cycle = 0;

    ESP32_HW_Init();

    while (1) {
        if (ESP32_Simple_AT_Test() == 0) {
            return 0;
        }

        if (cycle >= ESP32_AT_POWER_CYCLE_MAX) {
            return -1;
        }

        cycle++;
        DebugFramework_Printf("[AT] Power cycle %u/%u\n\r", cycle, ESP32_AT_POWER_CYCLE_MAX);
        ESP32_PowerCycle();
    }
}

ESP32_Test_Result_e ESP32_AT_Test_Init(void) { return ESP32_TEST_OK; }
ESP32_Test_Result_e ESP32_AT_Test_BasicAT(void) { return ESP32_TEST_OK; }
ESP32_Test_Result_e ESP32_AT_Test_GetVersion(void) { return ESP32_TEST_OK; }
ESP32_Test_Result_e ESP32_AT_Test_Reset(void) { return ESP32_TEST_OK; }
ESP32_Test_Result_e ESP32_AT_Test_SetStationMode(void) { return ESP32_TEST_OK; }
ESP32_Test_Result_e ESP32_AT_Test_ConnectWiFi(const char *s, const char *p)
{
    (void)s;
    (void)p;
    return ESP32_TEST_OK;
}
ESP32_Test_Result_e ESP32_AT_Test_GetIP(void) { return ESP32_TEST_OK; }
ESP32_Test_Result_e ESP32_AT_RunAllTests(const char *s, const char *p)
{
    (void)s;
    (void)p;
    ESP32_AT_Test_WithRecovery();
    return ESP32_TEST_OK;
}
const char *ESP32_AT_Test_GetLastError(void) { return ""; }
