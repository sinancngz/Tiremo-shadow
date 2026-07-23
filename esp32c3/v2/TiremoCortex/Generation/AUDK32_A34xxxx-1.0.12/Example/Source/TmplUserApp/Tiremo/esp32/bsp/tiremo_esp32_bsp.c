#include "abov_config.h"
#include "abov_module_config.h"

#include <string.h>

#include "hal_uart.h"
#include "hal_pcu.h"
#include "tiremo_esp32_bsp.h"
#include "tiremo_esp32_board.h"
#include "../../common/tiremo_systick.h"

static bool s_tiremoEsp32Ready = false;

bool TIREMO_ESP32_BSP_Init(void)
{
    if (s_tiremoEsp32Ready)
    {
        return true;
    }

    /* PA12 (power) and UART2 pins are configured in MCU Brew32. */
    TIREMO_ESP32_BSP_PowerOn();
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_BOOT_MS);

    s_tiremoEsp32Ready = true;
    return true;
}

void TIREMO_ESP32_BSP_PowerOn(void)
{
    (void)HAL_PCU_SetOutputBit(TIREMO_ESP32_PWR_PORT,
                               TIREMO_ESP32_PWR_PIN,
                               TIREMO_ESP32_PWR_ON);
}

void TIREMO_ESP32_BSP_PowerOff(void)
{
    (void)HAL_PCU_SetOutputBit(TIREMO_ESP32_PWR_PORT,
                               TIREMO_ESP32_PWR_PIN,
                               TIREMO_ESP32_PWR_OFF);
}

void TIREMO_ESP32_BSP_PowerCycle(void)
{
    TIREMO_ESP32_BSP_PowerOff();
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_OFF_MS);
    TIREMO_ESP32_BSP_PowerOn();
    TIREMO_SysTick_DelayMs(TIREMO_ESP32_PWR_BOOT_MS);
}

bool TIREMO_ESP32_BSP_UartTransmit(const uint8_t* data, uint32_t length)
{
    if ((data == NULL) || (length == 0U))
    {
        return false;
    }

    return (HAL_UART_Transmit(TIREMO_ESP32_UART_ID,
                              (uint8_t*)data,
                              length,
                              true) == HAL_ERR_OK);
}

bool TIREMO_ESP32_BSP_UartReceive(uint8_t* data, uint32_t maxLength)
{
    if ((data == NULL) || (maxLength == 0U))
    {
        return false;
    }

    return (HAL_UART_Receive(TIREMO_ESP32_UART_ID,
                             data,
                             maxLength,
                             true) == HAL_ERR_OK);
}

bool TIREMO_ESP32_BSP_SendAtCommand(const char* command, char* response, uint16_t respSize)
{
    HAL_ERR_e result;
    uint8_t txBuffer[64];
    uint32_t cmdLen;
    uint32_t rxLen;

    if ((command == NULL) || (response == NULL) || (respSize < 2U))
    {
        return false;
    }

    cmdLen = (uint32_t)strlen(command);
    if ((cmdLen + 2U) > sizeof(txBuffer))
    {
        return false;
    }

    memcpy(txBuffer, command, cmdLen);
    txBuffer[cmdLen] = (uint8_t)'\r';
    txBuffer[cmdLen + 1U] = (uint8_t)'\n';

    memset(response, 0, respSize);

    (void)HAL_UART_Transmit(TIREMO_ESP32_UART_ID, txBuffer, cmdLen + 2U, true);

    rxLen = TIREMO_ESP32_AT_RX_LEN;
    if (rxLen >= respSize)
    {
        rxLen = (uint32_t)respSize - 1U;
    }

    result = HAL_UART_Receive(TIREMO_ESP32_UART_ID,
                              (uint8_t*)response,
                              rxLen,
                              true);
    (void)result;

    return (strstr(response, "OK") != NULL);
}
